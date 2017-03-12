#include "filesys/cache.h"
#include <hash.h>
#include <stdbool.h>
#include <string.h>
#include "devices/block.h"
#include "filesys/filesys.h"
#include "threads/synch.h"
#include "threads/malloc.h"

/* List for clock eviction. */
static struct list cache_list;

/* Points to list_elem that is to be checked for eviction. */
static struct list_elem *clock_hand;

/* Global lock for protecting cache hash map. */
static struct lock cache_lock;

/* Handle global cache lock. */
static void acquire_cache_lock(void);
static void release_cache_lock(void);

/* Functions for hash implementation. */
static unsigned hash_sector_idx(const struct hash_elem *e, void *aux);
static bool hash_sector_less(const struct hash_elem *a,
        const struct hash_elem *b, void *aux);
static void hash_cache_free(struct hash_elem *e, void *aux);

static struct cache_sector *choose_sector_to_evict(void);
static void cache_remove(struct cache_sector *sector);
static void increment_clock_hand(void);

/*! Acquire global cache lock for modifying cache table/hashmap. */
static void acquire_cache_lock(void) {
    lock_acquire(&cache_lock);
}

/*! Release global cache lock for modifying cache table/hashmap. */
static void release_cache_lock(void) {
    lock_release(&cache_lock);
}

/*! Will initialize the global hash table. */
void cache_table_init(void) {
    hash_init(&cache_sector_table, hash_sector_idx, hash_sector_less, NULL);
    list_init(&cache_list);
    lock_init(&cache_lock);
    clock_hand = NULL;
}

/*! Free elements after they are removed from table. */
void cache_table_free(void) {
    hash_destroy(&cache_sector_table, hash_cache_free);
}

/*! Hash a cache_sector into the hash table based on its sector idx. */
static unsigned hash_sector_idx(const struct hash_elem *e, void *aux UNUSED) {
    struct cache_sector *sector = hash_entry(e, struct cache_sector,
            cache_hash_elem);
    int hash_idx = hash_int(sector->sector_idx);

    return hash_idx;
}

/*! Compares the sector indices for hash element comparisons */
static bool hash_sector_less(const struct hash_elem *a,
        const struct hash_elem *b, void *aux UNUSED) {
    struct cache_sector *first_sector = hash_entry(a, struct cache_sector,
            cache_hash_elem);
    struct cache_sector *second_sector = hash_entry(b, struct cache_sector,
            cache_hash_elem);

    return first_sector->sector_idx < second_sector->sector_idx;
}

/*! Frees the hash object from hash_table. */
static void hash_cache_free(struct hash_elem *e, void *aux UNUSED) {
    ASSERT(e != NULL);

    struct cache_sector *sector = hash_entry(e, struct cache_sector,
            cache_hash_elem);
    if (sector != NULL) {
        free(sector);
    }
    sector = NULL;
}

/*! Initialize a new sector_idx, and insert into hash table. */
struct cache_sector *cache_init(block_sector_t sector_idx, void *data) {
    /* First, check that it doesn't exist */
    ASSERT(cache_get(sector_idx) == NULL);
    struct cache_sector *new_sector = malloc(sizeof(struct cache_sector));

    new_sector->sector_idx = sector_idx;
    new_sector->accessed = false;
    new_sector->dirty = false;

    /* Read from memory into buffer */
    block_read(fs_device, sector_idx, data);

    hash_insert(&cache_sector_table, &new_sector->cache_hash_elem);
    if (clock_hand == NULL) {
        list_push_back(&cache_list, &new_sector->cache_list_elem);
    }
    else {
        list_insert(clock_hand, &new_sector->cache_list_elem);
    }
    return new_sector;
}

/*! Finds appropriate struct from hash table, and frees it and deletes it from
    table. */
void cache_free(block_sector_t sector_idx) {
    struct cache_sector *sector = cache_get(sector_idx);
    struct hash_elem *cache_sector_entry = hash_find(&cache_sector_table,
            &sector->cache_hash_elem);
    if (clock_hand == &sector->cache_list_elem) {
        increment_clock_hand();
    }
    list_remove(&sector->cache_list_elem);

    if (cache_sector_entry != NULL) {
        cache_sector_entry = hash_delete(&cache_sector_table,
                cache_sector_entry);

        ASSERT(cache_sector_entry != NULL);
        ASSERT(cache_get(sector_idx) == NULL);

        hash_cache_free(cache_sector_entry, NULL);
    }
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
    ASSERT(hash_size(&cache_sector_table) == MAX_BUFFER_SIZE);
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
void cache_insert(block_sector_t sector_idx, void *data) {
    /* Checks if the cache has already hit maximum capacity */
    if (hash_size(&cache_sector_table) == MAX_BUFFER_SIZE) {
        cache_evict();
    }

    ASSERT(hash_size(&cache_sector_table) < MAX_BUFFER_SIZE);
    cache_init(sector_idx, data);
}

/*! Will retrieve the specific cache from the cache map. */
struct cache_sector *cache_get(block_sector_t sector_idx) {
    struct cache_sector temp_sector;
    struct cache_sector *return_sector = NULL;
    temp_sector.sector_idx = sector_idx;

    struct hash_elem *cache_sector_entry = hash_find(&cache_sector_table,
            &temp_sector.cache_hash_elem);

    if (cache_sector_entry) {
        return_sector = hash_entry(cache_sector_entry, struct cache_sector,
                cache_hash_elem);
    }
    return return_sector;
}

/*! Will write data to a cache_sector buffer. */
void write_to_cache(block_sector_t sector_idx, void *data) {
    struct cache_sector *found_sector = cache_get(sector_idx);

    /* We want to be sure that the sector we find is not null */
    ASSERT(found_sector != NULL);
    memcpy(found_sector->sector, data, BLOCK_SECTOR_SIZE);

    found_sector->dirty = true;
}


/*! Will read from cache and write to memory. Involves freeing. */
void read_from_cache(block_sector_t sector_idx, void *data) {
    struct cache_sector *found_sector = cache_get(sector_idx);

    ASSERT(found_sector != NULL);
    memcpy(data, found_sector->sector, BLOCK_SECTOR_SIZE);
}
