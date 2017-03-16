#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/cache.h"
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"

/*! Identifies an inode. */
#define INODE_MAGIC 0x494e4f44

/*! On-disk inode.
    Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk {
    block_sector_t direct_blocks[DIRECT_BLOCK_COUNT];   /*!< Number of direct blocks. */
    block_sector_t indirect_block;                      /*!< Index for indirect block */
    block_sector_t double_indirect_block;               /*!< Index for double indirect block */

    off_t length;                                       /*!< File size in bytes. */
    unsigned magic;                                     /*!< Magic number. */
};

struct indirect_block {
    block_sector_t blocks[TOTAL_SECTOR_COUNT];          /*!< Number of direct blocks */
};

/*! Returns the number of sectors to allocate for an inode SIZE
    bytes long. */
static inline size_t bytes_to_sectors(off_t size) {
    return DIV_ROUND_UP(size, BLOCK_SECTOR_SIZE);
}

/*! In-memory inode. */
struct inode {
    struct list_elem elem;              /*!< Element in inode list. */
    block_sector_t sector;              /*!< Sector number of disk location. */
    int open_cnt;                       /*!< Number of openers. */
    bool removed;                       /*!< True if deleted, false otherwise. */
    int deny_write_cnt;                 /*!< 0: writes ok, >0: deny writes. */
    struct inode_disk data;             /*!< Inode content. */
};

static struct indirect_block *indirect_inode_new(void) {
    struct indirect_block *new_indir_block = calloc(1, sizeof(struct indirect_block));
    if (new_indir_block == NULL) {
        PANIC("Not enough memory for indirect inode block!");
    }
    return new_indir_block;
}

/* Handles populating the actual sectors pointed by the indirect block */
static bool handle_indirect_block(struct indirect_block *block, size_t num_indirect) {
    static char zeros[BLOCK_SECTOR_SIZE];
    unsigned idx;

    for (idx = 0; idx < num_indirect; idx++) {
        /* Ensure not allocated yet */
        ASSERT(block->blocks[idx] == 0);
        if (!free_map_allocate(1, &block->blocks[idx]))
            return false;
        write_to_cache(block->blocks[idx], zeros);
    }

    return true;
}

/* Handles the free-map allocations for direct blocks */
static bool handle_direct_alloc(struct inode_disk *disk, size_t num_direct) {
    static char zeros[BLOCK_SECTOR_SIZE];
    unsigned idx;

    /* We want to free map one at a time */
    for (idx = 0; idx < num_direct; idx++) {
        /* Ensure not allocated yet */
        ASSERT(disk->direct_blocks[idx] == 0);
        if (!free_map_allocate(1, &disk->direct_blocks[idx]))
            return false;
        write_to_cache(disk->direct_blocks[idx], zeros);
    }

    return true;
}

/* Handles the free-map allocations for indirect blocks. Will invoke handle_indirect_block */
static bool handle_indirect_alloc(struct inode_disk *disk, size_t num_indirect) {
    static char zeros[BLOCK_SECTOR_SIZE];

    if (num_indirect > 0) {
        struct indirect_block *new_indir_block = indirect_inode_new();

        /* If indirect block not allocated */
        if (disk->indirect_block == 0) {
            if (!free_map_allocate(1, &disk->indirect_block))
                return false;
            write_to_cache(disk->indirect_block, zeros);
        }

        /* Populate the pointers inside the indirect block */
        if (!handle_indirect_block(new_indir_block, num_indirect)) {
            free(new_indir_block);
            return false;
        }

        /* Write entire indirect block into cache */
        write_to_cache(disk->indirect_block, new_indir_block);
        free(new_indir_block);
    }

    return true;
}

/* Handles the free-map allocations for double indirect blocks. Will invoke
   handle_indirect_block several times */
static bool handle_double_indirect_alloc(struct inode_disk *disk, size_t num_double_indirect) {
    static char zeros[BLOCK_SECTOR_SIZE];

    /* If the file is actually large enough for a doubly indirect block... */
    if (num_double_indirect > 0) {
        /* Number of iterations we need to do */
        size_t indirect_blocks = DIV_ROUND_UP(num_double_indirect, TOTAL_SECTOR_COUNT);
        unsigned indirect_idx;


        /* If double indirect block not allocated, allocate that. */
        if (disk->double_indirect_block == 0) {
            if (!free_map_allocate(1, &disk->double_indirect_block))
                return false;
            write_to_cache(disk->double_indirect_block, zeros);
        }

        /* Create an doubly indirect block */
        struct indirect_block *new_double_indirect_block = indirect_inode_new();

        /* For each indirect block, we will essentially do what handle_indirect_alloc does */
        for (indirect_idx = 0; indirect_idx < indirect_blocks; indirect_idx++) {
            size_t num_sectors_in_indir = (num_double_indirect < TOTAL_SECTOR_COUNT) ? 
                                          num_double_indirect : TOTAL_SECTOR_COUNT;

            if (new_double_indirect_block->blocks[indirect_idx] == 0) {
                if (!free_map_allocate(1, &new_double_indirect_block->blocks[indirect_idx])) {
                    free(new_double_indirect_block);
                    return false;
                }

                write_to_cache(new_double_indirect_block->blocks[indirect_idx], zeros);
            }

            struct indirect_block *new_indir_block = indirect_inode_new();
            if (!handle_indirect_block(new_indir_block, num_sectors_in_indir)) {
                free(new_indir_block);
                free(new_double_indirect_block);
                return false;
            }

            /* Writing this indirect block information on one of the indices in the doubly indirect
               block */
            write_to_cache(new_double_indirect_block->blocks[indirect_idx], new_indir_block);
            free(new_indir_block);

            num_double_indirect -= num_sectors_in_indir;
        }

        ASSERT(num_double_indirect == 0);

        /* Doubly indirect block should be finished. Write all of that to inode_disk */
        write_to_cache(disk->double_indirect_block, new_double_indirect_block);
        free(new_double_indirect_block);
    }

    return true;
}

/* Based on the number of direct, indirect, and doubly indirect blocks, we
   map the appropriate sectors.

   Returns true if free_map was allocated properly given the disk
   Returns false otherwise. */
static bool inode_allocate_free_map(struct inode_disk *disk) {
    off_t length = disk->length;
    ASSERT(length >= 0);        /* We would hope that the length would be non-negative */

    size_t num_sectors = bytes_to_sectors(length);

    /* First, handle the direct blocks */
    size_t num_direct = (num_sectors < DIRECT_BLOCK_COUNT) ? num_sectors : DIRECT_BLOCK_COUNT;
    if (!handle_direct_alloc(disk, num_direct))
        return false;
    num_sectors -= num_direct;

    /* Next, handle the indirect block */
    size_t num_indirect = (num_sectors < TOTAL_SECTOR_COUNT) ? num_sectors : TOTAL_SECTOR_COUNT;
    if (!handle_indirect_alloc(disk, num_indirect))
        return false;
    num_sectors -= num_indirect;


    /* Finally, handle the doubly indirect block */
    size_t num_double_indirect = num_sectors;
    ASSERT(num_double_indirect < TOTAL_SECTOR_COUNT * TOTAL_SECTOR_COUNT);

    if (!handle_double_indirect_alloc(disk, num_double_indirect))
        return false;
    num_sectors -= num_double_indirect;

    /* All sectors should be accounted for */
    ASSERT(num_sectors == 0);
    return true;
}

/* This will release the bitmap in free_map when we're freeing an inode.
   Since we filled up inode direct -> indirect -> doubly indirect,
   we will release in the same order */
static void inode_release_free_map(struct inode_disk *disk) {
    unsigned idx;
    size_t length = disk->length;
    size_t num_sectors = bytes_to_sectors(length);

    /* First, release all direct block sectors */
    size_t num_direct = (num_sectors < DIRECT_BLOCK_COUNT) ? num_sectors : DIRECT_BLOCK_COUNT;
    for (idx = 0; idx < num_direct; idx++) {
        /* It's assumed that it has been allocated already */
        ASSERT(disk->direct_blocks[idx] != 0);
        free_map_release(disk->direct_blocks[idx], 1);
    }

    num_sectors -= num_direct;

    /* Next, release all indirect block sectors */
    size_t num_indirect = (num_sectors < TOTAL_SECTOR_COUNT) ? num_sectors : TOTAL_SECTOR_COUNT;
    struct indirect_block *temp_block = indirect_inode_new();
    if (num_indirect > 0) {
        read_from_cache(disk->indirect_block, temp_block);

        for (idx = 0; idx < num_indirect; idx++) {
            ASSERT(temp_block->blocks[idx] != 0);
            free_map_release(temp_block->blocks[idx], 1);
        }

        free_map_release(disk->indirect_block, 1);
    }
    free(temp_block);
    temp_block = NULL;

    num_sectors -= num_indirect;

    /* Finally, release all doubly indirect block sectors */
    size_t num_double_indirect = num_sectors;
    ASSERT(num_double_indirect < TOTAL_SECTOR_COUNT * TOTAL_SECTOR_COUNT);

    struct indirect_block *temp_double_block = indirect_inode_new();
    if (num_double_indirect > 0) {
        unsigned indirect_idx;
        /* Number of second layer indirects to account for... */
        size_t num_second_layer_blocks = DIV_ROUND_UP(num_double_indirect, TOTAL_SECTOR_COUNT);

        /* Writing the double indirect block struct into this pointer */
        ASSERT(disk->double_indirect_block != 0);
        read_from_cache(disk->double_indirect_block, temp_double_block);

        for (indirect_idx = 0; indirect_idx < num_second_layer_blocks; indirect_idx++) {
            ASSERT(temp_double_block->blocks[indirect_idx] != 0);
            struct indirect_block *second_layer_block = indirect_inode_new();

            read_from_cache(temp_double_block->blocks[indirect_idx], second_layer_block);
            
            size_t num_sectors_in_indir = (num_double_indirect < TOTAL_SECTOR_COUNT) ?
                                           num_double_indirect : TOTAL_SECTOR_COUNT;

            for (idx = 0; idx < num_sectors_in_indir; idx++) {
                ASSERT(second_layer_block->blocks[idx] != 0);
                free_map_release(second_layer_block->blocks[idx], 1);
            }

            free_map_release(temp_double_block->blocks[indirect_idx], 1);
            free(second_layer_block);
            num_double_indirect -= num_sectors_in_indir;
        }
        free_map_release(disk->double_indirect_block, 1);
    }
    free(temp_double_block);
}

/*! Returns the block device sector that contains byte offset POS
    within INODE.
    Returns -1 if INODE does not contain data for a byte at offset
    POS. */
// TODO: No longer contiguous. This might be annoying.
static block_sector_t byte_to_sector(const struct inode *inode, off_t pos) {
    ASSERT(inode != NULL);
    if (pos < inode->data.length)
        // return inode->data.start + pos / BLOCK_SECTOR_SIZE;
        return 0;
    return -1;
}

/*! List of open inodes, so that opening a single inode twice
    returns the same `struct inode'. */
static struct list open_inodes;

/*! Initializes the inode module. */
void inode_init(void) {
    list_init(&open_inodes);
}

/*! Initializes an inode with LENGTH bytes of data and
    writes the new inode to sector SECTOR on the file system
    device.
    Returns true if successful.
    Returns false if memory or disk allocation fails. */
bool inode_create(block_sector_t sector, off_t length) {
    struct inode_disk *disk_inode = NULL;
    bool success = false;

    ASSERT(length >= 0);

    /* If this assertion fails, the inode structure is not exactly
       one sector in size, and you should fix that. */
    ASSERT(sizeof *disk_inode == BLOCK_SECTOR_SIZE);

    disk_inode = calloc(1, sizeof *disk_inode);
    if (disk_inode != NULL) {
        disk_inode->length = length;
        disk_inode->magic = INODE_MAGIC;
        if (inode_allocate_free_map(disk_inode)) {
            write_to_cache(sector, disk_inode);
            success = true; 
        }
        free(disk_inode);
    }
    return success;
}

/*! Reads an inode from SECTOR
    and returns a `struct inode' that contains it.
    Returns a null pointer if memory allocation fails. */
struct inode * inode_open(block_sector_t sector) {
    struct list_elem *e;
    struct inode *inode;

    /* Check whether this inode is already open. */
    for (e = list_begin(&open_inodes); e != list_end(&open_inodes);
         e = list_next(e)) {
        inode = list_entry(e, struct inode, elem);
        if (inode->sector == sector) {
            inode_reopen(inode);
            return inode;
        }
    }

    /* Allocate memory. */
    inode = malloc(sizeof *inode);
    if (inode == NULL)
        return NULL;

    /* Initialize. */
    list_push_front(&open_inodes, &inode->elem);
    inode->sector = sector;
    inode->open_cnt = 1;
    inode->deny_write_cnt = 0;
    inode->removed = false;
    read_from_cache(inode->sector, &inode->data);
    return inode;
}

/*! Reopens and returns INODE. */
struct inode * inode_reopen(struct inode *inode) {
    if (inode != NULL)
        inode->open_cnt++;
    return inode;
}

/*! Returns INODE's inode number. */
block_sector_t inode_get_inumber(const struct inode *inode) {
    return inode->sector;
}

/*! Closes INODE and writes it to disk.
    If this was the last reference to INODE, frees its memory.
    If INODE was also a removed inode, frees its blocks. */
void inode_close(struct inode *inode) {
    /* Ignore null pointer. */
    if (inode == NULL)
        return;

    /* Release resources if this was the last opener. */
    if (--inode->open_cnt == 0) {
        /* Remove from inode list and release lock. */
        list_remove(&inode->elem);

        /* Deallocate blocks if removed. */
        if (inode->removed) {
            free_map_release(inode->sector, 1);
            inode_release_free_map(&inode->data);
        }

        free(inode);
    }
}

/*! Marks INODE to be deleted when it is closed by the last caller who
    has it open. */
void inode_remove(struct inode *inode) {
    ASSERT(inode != NULL);
    inode->removed = true;
}

/*! Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t inode_read_at(struct inode *inode, void *buffer_, off_t size, off_t offset) {
    uint8_t *buffer = buffer_;
    off_t bytes_read = 0;

    while (size > 0) {
        /* Disk sector to read, starting byte offset within sector. */
        block_sector_t sector_idx = byte_to_sector (inode, offset);
        int sector_ofs = offset % BLOCK_SECTOR_SIZE;

        /* Bytes left in inode, bytes left in sector, lesser of the two. */
        off_t inode_left = inode_length(inode) - offset;
        int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
        int min_left = inode_left < sector_left ? inode_left : sector_left;

        /* Number of bytes to actually copy out of this sector. */
        int chunk_size = size < min_left ? size : min_left;
        if (chunk_size <= 0)
            break;

        read_cache_offset(sector_idx, buffer + bytes_read, sector_ofs,
                chunk_size);

        /* Advance. */
        size -= chunk_size;
        offset += chunk_size;
        bytes_read += chunk_size;
    }

    return bytes_read;
}

/*! Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
    Returns the number of bytes actually written, which may be
    less than SIZE if end of file is reached or an error occurs.
    (Normally a write at end of file would extend the inode, but
    growth is not yet implemented.) */

// TODO: Handle EOF extensions.
off_t inode_write_at(struct inode *inode, const void *buffer_, off_t size, off_t offset) {
    const uint8_t *buffer = buffer_;
    off_t bytes_written = 0;

    if (inode->deny_write_cnt)
        return 0;

    while (size > 0) {
        /* Sector to write, starting byte offset within sector. */
        block_sector_t sector_idx = byte_to_sector(inode, offset);
        int sector_ofs = offset % BLOCK_SECTOR_SIZE;

        /* Bytes left in inode, bytes left in sector, lesser of the two. */
        off_t inode_left = inode_length(inode) - offset;
        int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
        int min_left = inode_left < sector_left ? inode_left : sector_left;

        /* Number of bytes to actually write into this sector. */
        int chunk_size = size < min_left ? size : min_left;
        if (chunk_size <= 0)
            break;

        write_cache_offset(sector_idx, buffer + bytes_written, sector_ofs,
            chunk_size);

        /* Advance. */
        size -= chunk_size;
        offset += chunk_size;
        bytes_written += chunk_size;
    }

    return bytes_written;
}

/*! Disables writes to INODE.
    May be called at most once per inode opener. */
void inode_deny_write (struct inode *inode) {
    inode->deny_write_cnt++;
    ASSERT(inode->deny_write_cnt <= inode->open_cnt);
}

/*! Re-enables writes to INODE.
    Must be called once by each inode opener who has called
    inode_deny_write() on the inode, before closing the inode. */
void inode_allow_write (struct inode *inode) {
    ASSERT(inode->deny_write_cnt > 0);
    ASSERT(inode->deny_write_cnt <= inode->open_cnt);
    inode->deny_write_cnt--;
}

/*! Returns the length, in bytes, of INODE's data. */
off_t inode_length(const struct inode *inode) {
    return inode->data.length;
}

