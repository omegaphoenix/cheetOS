#include "filesys/directory.h"
#include <stdio.h>
#include <string.h>
#include <list.h>
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "threads/malloc.h"
#include "threads/synch.h"

/*! A directory. */
struct dir {
    struct inode *inode;                /*!< Backing store. */
    off_t pos;                          /*!< Current position. */
    struct lock dir_lock;               /*!< Lock for I/O operations. */
    struct list_elem elem;              /*!< Element in inode list. */
    int open_cnt;                       /*!< Number of times dir has been opened. */
};

/*! A single directory entry. */
struct dir_entry {
    block_sector_t inode_sector;        /*!< Sector number of header. */
    char name[NAME_MAX + 1];            /*!< Null terminated file name. */
    bool in_use;                        /*!< In use or free? */
};

static void acquire_dir_lock(struct dir *dir) {
    lock_acquire(&dir->dir_lock);
}

static void release_dir_lock(struct dir *dir) {
    lock_release(&dir->dir_lock);
}

/*! List of open directories, so that opening a single directory twice
    returns the same `struct dir'. */
static struct list open_dirs;

/*! Initializes the inode module. */
void directory_init(void) {
    list_init(&open_dirs);
}

/*! Creates a directory with space for ENTRY_CNT entries in the
    given SECTOR.  Returns true if successful, false on failure. */
bool dir_create(block_sector_t sector, size_t entry_cnt) {
    return inode_create(sector, entry_cnt * sizeof(struct dir_entry));
}

/*! Opens and returns the directory for the given INODE, of which
    it takes ownership.  Returns a null pointer on failure. */
struct dir * dir_open(struct inode *inode) {
    struct list_elem *e;
    struct dir *dir;

    /* Check if inode is already open as dir */
    for (e = list_begin(&open_dirs); e != list_end(&open_dirs);
         e = list_next(e)) {
        dir = list_entry(e, struct dir, elem);
        if (dir_get_inode(dir) == inode) {
            dir->open_cnt++;
            inc_in_use(inode);
            return dir;
        }
    }

    /* Couldn't find open dir. */
    dir = calloc(1, sizeof(*dir));
    if (inode != NULL && dir != NULL) {
        list_push_front(&open_dirs, &dir->elem);
        dir->inode = inode;
        dir->pos = 0;
        lock_init(&dir->dir_lock);
        dir->open_cnt++;
        inc_in_use(dir->inode);
        return dir;
    }
    else {
        inode_close(inode);
        free(dir);
        return NULL;
    }
}

/*! Opens the root directory and returns a directory for it.
    Return true if successful, false on failure. */
struct dir * dir_open_root(void) {
    return dir_open(inode_open(ROOT_DIR_SECTOR));
}

/* Opens and returns a new directory for the same inode as DIR.
   Returns a null pointer on failure. */
struct dir * dir_reopen(struct dir *dir) {
    return dir_open(inode_reopen(dir->inode));
}

/*! Destroys DIR and frees associated resources. */
void dir_close(struct dir *dir) {
    if (dir != NULL) {
        dec_in_use(dir->inode);
        dir->open_cnt--;
        if(dir->open_cnt == 0) {
            /* Release resources if this was the last opener. */
            list_remove(&dir->elem);
            inode_close(dir->inode);
            free(dir);
        }

    }
}

/*! Returns the inode encapsulated by DIR. */
struct inode * dir_get_inode(struct dir *dir) {
    return dir->inode;
}

/*! Searches DIR for a file with the given NAME.
    If successful, returns true, sets *EP to the directory entry
    if EP is non-null, and sets *OFSP to the byte offset of the
    directory entry if OFSP is non-null.
    otherwise, returns false and ignores EP and OFSP. */
static bool lookup(const struct dir *dir, const char *name,
                   struct dir_entry *ep, off_t *ofsp) {
    struct dir_entry e;
    size_t ofs;

    ASSERT(dir != NULL);
    ASSERT(name != NULL);

    for (ofs = 0; inode_read_at(dir->inode, &e, sizeof(e), ofs) == sizeof(e);
         ofs += sizeof(e)) {
        if (e.in_use && !strcmp(name, e.name)) {
            if (ep != NULL)
                *ep = e;
            if (ofsp != NULL)
                *ofsp = ofs;
            return true;
        }
    }
    return false;
}

/*! Searches DIR for a file with the given NAME and returns true if one exists,
    false otherwise.  On success, sets *INODE to an inode for the file,
    otherwise to a null pointer.  The caller must close *INODE. */
bool dir_lookup(const struct dir *dir, const char *name, struct inode **inode) {

    struct dir_entry e;

    ASSERT(dir != NULL);
    ASSERT(name != NULL);

    if (lookup(dir, name, &e, NULL))
        *inode = inode_open(e.inode_sector);
    else
        *inode = NULL;

    return *inode != NULL;
}

/*! Adds a file named NAME to DIR, which must not already contain a file by
    that name.  The file's inode is in sector INODE_SECTOR.
    Returns true if successful, false on failure.
    Fails if NAME is invalid (i.e. too long) or a disk or memory
    error occurs. */
bool dir_add(struct dir *dir, const char *name, block_sector_t inode_sector) {
    struct dir_entry e;
    off_t ofs;
    bool success = false;

    ASSERT(dir != NULL);
    ASSERT(name != NULL);

    /* Check NAME for validity. */
    if (*name == '\0' || strlen(name) > NAME_MAX)
        return false;

    acquire_dir_lock(dir);
    /* Check that NAME is not in use. */
    if (lookup(dir, name, NULL, NULL))
        goto done;

    /* Set OFS to offset of free slot.
       If there are no free slots, then it will be set to the
       current end-of-file.

       inode_read_at() will only return a short read at end of file.
       Otherwise, we'd need to verify that we didn't get a short
       read due to something intermittent such as low memory. */
    for (ofs = 0; inode_read_at(dir->inode, &e, sizeof(e), ofs) == sizeof(e);
         ofs += sizeof(e)) {
        if (!e.in_use)
            break;
    }

    /* Write slot. */
    e.in_use = true;
    strlcpy(e.name, name, sizeof e.name);
    e.inode_sector = inode_sector;
    success = inode_write_at(dir->inode, &e, sizeof(e), ofs) == sizeof(e);

done:
    release_dir_lock(dir);
    return success;
}

/*! Removes any entry for NAME in DIR.  Returns true if successful, false on
    failure, which occurs only if there is no file with the given NAME. */
bool dir_remove(struct dir *dir, const char *name) {
    struct dir_entry e;
    struct inode *inode = NULL;
    bool success = false;
    off_t ofs;

    ASSERT(dir != NULL);
    ASSERT(name != NULL);

    acquire_dir_lock(dir);
    /* Find directory entry. */
    if (!lookup(dir, name, &e, &ofs))
        goto done;

    /* Open inode. */
    inode = inode_open(e.inode_sector);
    if (inode == NULL)
        goto done;

    /* If directory, need additional checks. */
    if (is_dir(inode)) {
        /* Criteria for deleting a directory:
           - dir must be empty to be deleted.
           - dir must not be in use as working directory or in a process. */
        if (!is_empty_dir(inode) || get_in_use(inode) > 0) {
            success = false;
            goto done;
        }
    }

    /* Erase directory entry. */
    e.in_use = false;
    if (inode_write_at(dir->inode, &e, sizeof(e), ofs) != sizeof(e))
        goto done;

    /* Remove inode. */
    inode_remove(inode);
    success = true;

done:
    inode_close(inode);
    release_dir_lock(dir);
    return success;
}

/*! Reads the next directory entry in DIR and stores the name in NAME.  Returns
    true if successful, false if the directory contains no more entries.
    Ignore "." and ".." entries. */
bool dir_readdir(struct dir *dir, char name[NAME_MAX + 1]) {
    struct dir_entry e;

    acquire_dir_lock(dir);
    while (inode_read_at(dir->inode, &e, sizeof(e), dir->pos) == sizeof(e)) {
        dir->pos += sizeof(e);
        if (e.in_use && strcmp(e.name, ".") && strcmp(e.name, "..")) {
            strlcpy(name, e.name, NAME_MAX + 1);
            release_dir_lock(dir);
            return true;
        }
    }
    release_dir_lock(dir);
    return false;
}

/* Returns true if directory is empty (other than "." and "..") */
bool is_empty_dir(struct inode *inode) {
    struct dir *dir = dir_open(inode_reopen(inode));
    struct dir_entry e;
    size_t ofs;

    ASSERT(is_dir(inode));
    ASSERT(dir != NULL);

    for (ofs = 0; inode_read_at(dir->inode, &e, sizeof(e), ofs) == sizeof(e);
         ofs += sizeof(e)) {
        if (e.in_use && strcmp(e.name, ".") && strcmp(e.name, "..")) {
            dir_close(dir);
            return false;
        }
    }
    dir_close(dir);
    return true;
}

/* Returns previously opened directory with given inode */
struct dir *get_open_dir(struct inode *inode) {
    struct list_elem *e;
    struct dir *dir;

    for (e = list_begin(&open_dirs); e != list_end(&open_dirs);
         e = list_next(e)) {
        dir = list_entry(e, struct dir, elem);
        if (dir_get_inode(dir) == inode) {
            return dir;
        }
    }
    return NULL;
}

/* Initializes subdirectory. */
bool init_subdir(struct inode *inode, struct dir *parent_dir) {
    bool success = true;
    /* Set inode's 'is_dir' to true */
    set_dir(inode, true);

    /* Add "." and ".." directories */
    struct dir *dir = dir_open(inode_reopen(inode));
    /* "." points to itself */
    success = dir_add(dir, ".", inode_get_inumber(inode));

    if (inode_get_inumber(inode) == ROOT_DIR_SECTOR) {
         /* ".." points to itself */
        success = dir_add(dir, "..", inode_get_inumber(inode));
    }
    else {
        /* ".." points to parent */
        success = dir_add(dir, "..", inode_get_inumber(dir_get_inode(parent_dir)));
    }

    dir_close(dir);
    return success;
}
