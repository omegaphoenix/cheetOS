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

static int cache_get_free(void);
static bool in_cache(block_sector_t idx);
static struct cache_sector *choose_sector_to_evict(void);
static void cache_remove(struct cache_sector *sector);
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
    }
}

/*! Return true if idx is stored in the buffer cache. */
static bool in_cache(block_sector_t idx) {
    int i;
    for (i = 0; i < MAX_BUFFER_SIZE; i++) {
        if (cache_buffer[i].valid && cache_buffer[i].sector_idx == idx) {
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
struct cache_sector *cache_init(block_sector_t sector_idx) {
    /* First, check that it doesn't exist */
    ASSERT(!in_cache(sector_idx));
    int i = cache_get_free();
    ASSERT(i != -1);
    cache_buffer[i].sector_idx = sector_idx;
    cache_buffer[i].valid = true;
    cache_buffer[i].accessed = true;
    cache_buffer[i].dirty = false;

    /* Read from memory into buffer */
    block_read(fs_device, sector_idx, cache_buffer[i].sector);

    /* Insert into eviction list. */
    if (clock_hand == NULL) {
        list_push_back(&cache_list, &cache_buffer[i].cache_list_elem);
    }
    else {
        list_insert(clock_hand, &cache_buffer[i].cache_list_elem);
    }
    return &cache_buffer[i];
}

/*! Finds appropriate sector and removes it from table. */
void cache_free(block_sector_t sector_idx) {
    int i = cache_get(sector_idx);
    if (clock_hand == &cache_buffer[i].cache_list_elem) {
        increment_clock_hand();
    }
    cache_buffer[i].valid = false;
    list_remove(&cache_buffer[i].cache_list_elem);
}

/*! Increment clock hand in clock algorithm. */
static void increment_clock_hand(void) {
    if (clock_hand == NULL || clock_hand == list_end(&cache_list)) {
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
static struct cache_sector *choose_sector_to_evict(void) {
    ASSERT(!list_empty(&cache_list));
    if (clock_hand == NULL) {
        increment_clock_hand();
    }
    struct cache_sector *sector =
        list_entry(clock_hand, struct cache_sector, cache_list_elem);

    while (sector->accessed) {
        sector->accessed = false;

        increment_clock_hand();
        sector = list_entry(clock_hand, struct cache_sector, cache_list_elem);
    }

    increment_clock_hand();
    return sector;
}

/*! Uses block algorithm to choose the next block to evict. */
void cache_evict(void) {
    struct cache_sector *sector_to_evict = choose_sector_to_evict();
    cache_remove(sector_to_evict);

    /* Free memory. */
    cache_free(sector_to_evict->sector_idx);
}

/*! Removes a block from the buffer cache. This involves it writing from cache
    back to memory. */
static void cache_remove(struct cache_sector *sector) {
    block_write(fs_device, sector->sector_idx, sector->sector);
    sector->dirty = false;
}

/*! Adds a block into buffer cache. Evicts if necessary. Will write from
    memory to cache. */
struct cache_sector *cache_insert(block_sector_t sector_idx) {
    /* Checks if the cache has already hit maximum capacity */
    if (is_full_cache()) {
        cache_evict();
    }

    ASSERT(!is_full_cache());
    ASSERT(list_size(&cache_list) < MAX_BUFFER_SIZE);
    return cache_init(sector_idx);
}

/*! Will retrieve the specific cache from the cache map. */
int cache_get(block_sector_t idx) {
    int i;
    int ret = -1;
    for (i = 0; i < MAX_BUFFER_SIZE; i++) {
        if (cache_buffer[i].valid && cache_buffer[i].sector_idx == idx) {
            ret = i;
        }
    }
    return ret;
}

/*! Will retrieve the specific cache from the cache map. */
static int cache_get_free(void) {
    int i;
    for (i = 0; i < MAX_BUFFER_SIZE; i++) {
        if (!cache_buffer[i].valid) {
            return i;
        }
    }
    return -1;
}

/*! Will write data to a cache_sector buffer. */
void write_to_cache(block_sector_t sector_idx, void *data) {
#ifdef CACHE
    int found_idx = cache_get(sector_idx);
    struct cache_sector *found_sector;

    if (found_idx == -1) {
        /* Import sector into cache. */
        found_sector = cache_insert(sector_idx);
    }
    else {
        found_sector= &cache_buffer[found_idx];
    }

    /* We want to be sure that the sector we find is not null */
    ASSERT(found_sector != NULL);
    memcpy(found_sector->sector, data, BLOCK_SECTOR_SIZE);

    found_sector->accessed = true;
    found_sector->dirty = true;
#else
    block_write(fs_device, sector_idx, data);
#endif
}

/*! Will read from cache and write to memory. Involves freeing. */
void read_from_cache(block_sector_t sector_idx, void *data) {
#ifdef CACHE
    int found_idx = cache_get(sector_idx);
    struct cache_sector *found_sector;

    if (found_idx == -1) {
        /* Import sector into cache. */
        found_sector = cache_insert(sector_idx);
    }
    else {
        found_sector= &cache_buffer[found_idx];
    }

    ASSERT(found_sector != NULL);
    memcpy(data, found_sector->sector, BLOCK_SECTOR_SIZE);

    found_sector->accessed = true;
#else
    block_read(fs_device, sector_idx, data);
#endif
}

/*! Will read from cache and write to memory. Involves freeing. */
void read_cache_offset(block_sector_t sector_idx, void *data, off_t ofs,
        size_t bytes) {
    ASSERT(ofs >= 0 && ofs < BLOCK_SECTOR_SIZE);
    ASSERT(bytes > 0 && bytes <= BLOCK_SECTOR_SIZE);
#ifdef NCACHE
    int found_idx = cache_get(sector_idx);
    struct cache_sector *found_sector;

    if (found_idx == -1) {
        /* Import sector into cache. */
        found_sector = cache_insert(sector_idx);
    }
    else {
        found_sector = &cache_buffer[found_idx];
    }
    ASSERT(found_sector != NULL);
    memcpy(data, found_sector->sector + ofs, bytes);

    found_sector->accessed = true;
#else
    uint8_t *bounce = malloc(BLOCK_SECTOR_SIZE);
    if (bounce == NULL) {
        return;
    }
    block_read(fs_device, sector_idx, bounce);
    memcpy(data, bounce + ofs, bytes);
#endif
}
