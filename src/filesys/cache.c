#include "filesys/cache.h"
#include <stdbool.h>
#include <string.h>
#include "devices/block.h"
#include "filesys/filesys.h"
#include "filesys/off_t.h"
#include "threads/synch.h"
#include "threads/malloc.h"

static struct cache_sector cache_buffer[MAX_BUFFER_SIZE];

/* List for clock eviction. */
static struct list cache_list;

/* Points to list_elem that is to be checked for eviction. */
static struct list_elem *clock_hand;

/* Global lock for protecting cache table. */
static struct lock cache_lock;

/* Handle global cache lock. */
static void acquire_cache_lock(void);
static void release_cache_lock(void);

/* cache_sector constructor/destructor. Removal/insertion into list. */
static int cache_init(block_sector_t sector_idx);
static void cache_free(block_sector_t sector_idx);

/* Might use clock algorithm for this. */
static void cache_evict(void);

/* Insertion from buffer cache. */
static int cache_insert(block_sector_t sector_idx);

/* Retrieve cache_buffer index that corresponds to block sector_idx. */
static int cache_get(block_sector_t sector_idx);

static int cache_get_free(void);
static bool in_cache(block_sector_t sector_idx);
static int choose_sector_to_evict(void);
static void cache_remove(int array_idx);
static void increment_clock_hand(void);

/*! Acquire global cache lock for modifying cache table. */
static void acquire_cache_lock(void) {
    lock_acquire(&cache_lock);
}

/*! Release global cache lock for modifying cache table. */
static void release_cache_lock(void) {
    lock_release(&cache_lock);
}

/*! Will initialize the global variables. */
void cache_table_init(void) {
    list_init(&cache_list);
    lock_init(&cache_lock);
    clock_hand = NULL;
    int i;
    for (i = 0; i < MAX_BUFFER_SIZE; i++) {
        cache_buffer[i].sector_idx = 0;
        cache_buffer[i].valid = false;
        cache_buffer[i].accessed = false;
        cache_buffer[i].dirty = false;
        cache_buffer[i].pin_count = 0;
        rw_lock_init(&cache_buffer[i].read_write_lock);
    }
}

static void pin(int array_idx) {
    cache_buffer[array_idx].pin_count++;
}

static void unpin(int array_idx) {
    ASSERT(cache_buffer[array_idx].valid);
    ASSERT(cache_buffer[array_idx].pin_count > 0);
    cache_buffer[array_idx].pin_count--;
}

/*! Return true if idx is stored in the buffer cache. */
static bool in_cache(block_sector_t sector_idx) {
    int i;
    for (i = 0; i < MAX_BUFFER_SIZE; i++) {
        if (cache_buffer[i].valid
                && cache_buffer[i].sector_idx == sector_idx) {
            return true;
        }
    }
    return false;
}

/*! Return true if cache is full. */
static bool is_full_cache(void) {
    int i;
    for (i = 0; i < MAX_BUFFER_SIZE; i++) {
        if (!cache_buffer[i].valid) {
            return false;
        }
    }
    return true;
}

/*! Initialize a new sector_idx, and insert into cache buffer. */
static int cache_init(block_sector_t sector_idx) {
    /* First, check that it doesn't exist */
    ASSERT(!in_cache(sector_idx));
    int i = cache_get_free();
    ASSERT(i != -1);
    cache_buffer[i].sector_idx = sector_idx;
    cache_buffer[i].valid = true;
    cache_buffer[i].accessed = true;
    cache_buffer[i].dirty = false;

    /* Read from memory into buffer */
    begin_write(&cache_buffer[i].read_write_lock);
    block_read(fs_device, sector_idx, cache_buffer[i].sector);
    end_write(&cache_buffer[i].read_write_lock);

    /* Insert into eviction list. */
    if (clock_hand == NULL) {
        list_push_back(&cache_list, &cache_buffer[i].cache_list_elem);
    }
    else {
        list_insert(clock_hand, &cache_buffer[i].cache_list_elem);
    }
    return i;
}

/*! Finds appropriate sector and removes it from table. */
static void cache_free(block_sector_t sector_idx) {
    int i = cache_get(sector_idx);
    ASSERT(i != -1);
    ASSERT(cache_buffer[i].pin_count == 1);
    ASSERT(cache_buffer[i].valid);
    if (clock_hand == &cache_buffer[i].cache_list_elem) {
        increment_clock_hand();
    }
    cache_buffer[i].valid = false;
    cache_buffer[i].accessed = false;
    cache_buffer[i].dirty = false;
    cache_buffer[i].sector_idx = 0;
    cache_buffer[i].pin_count = 0;
    list_remove(&cache_buffer[i].cache_list_elem);
}

/*! Increment clock hand in clock algorithm. */
static void increment_clock_hand(void) {
    if (clock_hand == NULL || clock_hand == list_back(&cache_list)) {
        clock_hand = list_begin(&cache_list);
    }
    else {
        clock_hand = list_next(clock_hand);
    }

    if (list_size(&cache_list) == 1) {
        clock_hand = NULL;
    }
}

/*! Choose a cache sector to be evicted based on clock algorithm. */
static int choose_sector_to_evict(void) {
    ASSERT(!list_empty(&cache_list));
    if (clock_hand == NULL) {
        increment_clock_hand();
    }
    struct cache_sector *sector =
        list_entry(clock_hand, struct cache_sector, cache_list_elem);

    while (sector->accessed || sector->pin_count > 0) {
        if (sector->pin_count == 0) {
            sector->accessed = false;
        }
        increment_clock_hand();
        sector = list_entry(clock_hand, struct cache_sector, cache_list_elem);
    }
    increment_clock_hand();
    return sector->sector_idx;
}

/*! Uses block algorithm to choose the next block to evict. */
static void cache_evict(void) {
    int sector_idx = choose_sector_to_evict();
    cache_remove(sector_idx);

    /* Free memory. */
    cache_free(sector_idx);
}

/*! Removes a block from the buffer cache. This involves it writing from cache
    back to memory. */
static void cache_remove(int sector_idx) {
    int array_idx = cache_get(sector_idx);
    begin_write(&cache_buffer[array_idx].read_write_lock);
    block_write(fs_device, sector_idx, cache_buffer[array_idx].sector);
    end_write(&cache_buffer[array_idx].read_write_lock);
    cache_buffer[array_idx].dirty = false;
}

/*! Adds a block into buffer cache. Evicts if necessary. Will write from
    memory to cache. */
static int cache_insert(block_sector_t sector_idx) {
    /* Checks if the cache has already hit maximum capacity */
    if (is_full_cache()) {
        cache_evict();
    }

    ASSERT(!is_full_cache());
    ASSERT(list_size(&cache_list) < MAX_BUFFER_SIZE);
    return cache_init(sector_idx);
}

/* Retrieve cache_buffer index that corresponds to block sector_idx. */
static int cache_get(block_sector_t sector_idx) {
    int i;
    for (i = 0; i < MAX_BUFFER_SIZE; i++) {
        if (cache_buffer[i].valid
                && cache_buffer[i].sector_idx == sector_idx) {
            pin(i);
            return i;
        }
    }
    return -1;
}

/*! Will retrieve the specific cache from the cache map. */
static int cache_get_free(void) {
    int i;
    for (i = 0; i < MAX_BUFFER_SIZE; i++) {
        if (!cache_buffer[i].valid) {
            pin(i);
            return i;
        }
    }
    return -1;
}

/*! Write data to a cache_sector buffer. */
void write_to_cache(block_sector_t sector_idx, const void *data) {
#ifdef CACHE
    write_cache_offset(sector_idx, data, 0, BLOCK_SECTOR_SIZE);
#else
    block_write(fs_device, sector_idx, data);
#endif
}

/* Write data to cache_sector buffer at an offset. */
void write_cache_offset(block_sector_t sector_idx, const void *data, off_t ofs,
    size_t bytes) {
    ASSERT(ofs >= 0 && ofs < BLOCK_SECTOR_SIZE);
    ASSERT(bytes > 0 && bytes <= BLOCK_SECTOR_SIZE);
#ifdef CACHE
    acquire_cache_lock();
    int idx = cache_get(sector_idx);

    if (idx == -1) {
        /* Import sector into cache. */
        idx = cache_insert(sector_idx);
    }

    /* We want to be sure that the sector we find is not null */
    ASSERT(cache_buffer[idx].valid);
    begin_write(&cache_buffer[idx].read_write_lock);
    memcpy(cache_buffer[idx].sector + ofs, data, bytes);
    end_write(&cache_buffer[idx].read_write_lock);

    cache_buffer[idx].accessed = true;
    cache_buffer[idx].dirty = true;
    unpin(idx);
    release_cache_lock();
#else
    /* We need a bounce buffer. */
    uint8_t *bounce = malloc(BLOCK_SECTOR_SIZE);
    if (bounce == NULL) {
        return;
    }

    /* If the sector contains data before or after the chunk
       we're writing, then we need to read in the sector
       first.  Otherwise we start with a sector of all zeros. */
    size_t sector_left = BLOCK_SECTOR_SIZE - ofs;
    if (ofs > 0 || bytes < sector_left)
        block_read(fs_device, sector_idx, bounce);
    else
        memset(bounce, 0, BLOCK_SECTOR_SIZE);

    memcpy(bounce + ofs, data, bytes);
    block_write(fs_device, sector_idx, bounce);
#endif
}

/*! Read from cache and write to memory. Involves freeing. */
void read_from_cache(block_sector_t sector_idx, void *data) {
#ifdef CACHE
    read_cache_offset(sector_idx, data, 0, BLOCK_SECTOR_SIZE);
#else
    block_read(fs_device, sector_idx, data);
#endif
}

/*! Read from cache at an offset and write to memory. Involves freeing. */
void read_cache_offset(block_sector_t sector_idx, void *data, off_t ofs,
        size_t bytes) {
    ASSERT(ofs >= 0 && ofs < BLOCK_SECTOR_SIZE);
    ASSERT(bytes > 0 && bytes <= BLOCK_SECTOR_SIZE);
#ifdef CACHE
    acquire_cache_lock();
    int idx = cache_get(sector_idx);

    if (idx == -1) { // if we load the cache every time, it works.
        /* Import sector into cache. */
        idx = cache_insert(sector_idx);
    }

    ASSERT(cache_buffer[idx].valid);
    begin_read(&cache_buffer[idx].read_write_lock);
    memcpy(data, cache_buffer[idx].sector + ofs, bytes);
    end_read(&cache_buffer[idx].read_write_lock);

    cache_buffer[idx].accessed = true;
    unpin(idx);
    release_cache_lock();
#else
    /* Read sector into bounce buffer, then partially copy
       into caller's buffer. */
    uint8_t *bounce = malloc(BLOCK_SECTOR_SIZE);
    if (bounce == NULL) {
        return;
    }
    block_read(fs_device, sector_idx, bounce);
    memcpy(data, bounce + ofs, bytes);
#endif
}
