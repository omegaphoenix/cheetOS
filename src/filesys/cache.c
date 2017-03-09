#include <hash.h>
#include <stdbool.h>
#include <string.h>

#include "cache.h"
#include "devices/block.h"
#include "filesys/filesys.h"
#include "threads/synch.h"
#include "threads/malloc.h"

static unsigned hash_sector_idx(const struct hash_elem *e, void *aux);
static bool hash_sector_less(const struct hash_elem *a, const struct hash_elem *b, void *aux);
static void hash_cache_free(struct hash_elem *e, void *aux);

/* Will initialize the global hash table. */
void cache_table_init(void) {
    hash_init(&cache_sector_table, hash_sector_idx, hash_sector_less, NULL);
    lock_init(&cache_lock);
}

/* Frees elements after they are removed from table */
void cache_table_free(void) {
    // TODO: I'm not too sure when to free this tbh.
    hash_destroy(&cache_sector_table, hash_cache_free);
}

/* Hashes a cache_sector into the hash table based on its sector idx */
static unsigned hash_sector_idx(const struct hash_elem *e, void *aux UNUSED) {
    struct cache_sector *sector = hash_entry(e, struct cache_sector, cache_sector_elem);
    int hash_idx = hash_int(sector->sector_idx);

    return hash_idx;
}

/* Compares the sector indices for hash element comparisons */
static bool hash_sector_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
    struct cache_sector *first_sector = hash_entry(a, struct cache_sector, cache_sector_elem);
    struct cache_sector *second_sector = hash_entry(b, struct cache_sector, cache_sector_elem);

    return first_sector->sector_idx < second_sector->sector_idx;
}

/* Frees the hash object from hash_table. */
static void hash_cache_free(struct hash_elem *e, void *aux UNUSED) {
    ASSERT(e != NULL);

    struct cache_sector *sector = hash_entry(e, struct cache_sector, cache_sector_elem);
    if (sector != NULL) {
        free(sector);
    }
    sector = NULL;
}

/* Initialize a new sector_idx, and insert into hash table. */
struct cache_sector *cache_init(block_sector_t sector_idx, void *data) {
    /* First, check that it doesn't exist */
    ASSERT(cache_get(sector_idx) == NULL);
    struct cache_sector *new_sector = malloc(sizeof(struct cache_sector));

    new_sector->sector_idx = sector_idx;
    new_sector->access_level = NOT_ACCESSED;
    new_sector->is_dirty = false;

    /* Read from memory into buffer */
    block_read(fs_device, sector_idx, data);

    lock_acquire(&cache_lock);
    hash_insert(&cache_sector_table, &new_sector->cache_sector_elem);
    lock_release(&cache_lock);
    return new_sector;
}

/* Finds appropriate struct from hash table, and frees it/deletes it from table */
void cache_free(block_sector_t sector_idx) {
    struct cache_sector temp_sector;
    temp_sector.sector_idx = sector_idx;
    lock_acquire(&cache_lock);
    struct hash_elem *cache_sector_entry = hash_find(&cache_sector_table, &temp_sector.cache_sector_elem);

    if (cache_sector_entry) {
        cache_sector_entry = hash_delete(&cache_sector_table, cache_sector_entry);

        ASSERT(cache_sector_entry != NULL);
        ASSERT(cache_get(sector_idx) == NULL);

        hash_cache_free(cache_sector_entry, NULL);
    }
    lock_release(&cache_lock);
}

/* Uses block algorithm to choose the next block to evict */
void cache_evict(void) {
  // TODO
}

/* Removes a block from the buffer cache. This involves it writing from cache back
   to memory */
void cache_remove(block_sector_t sector_idx) {
  // TODO
}

/* Adds a block into buffer cache. Evicts if necessary. Will write from
   memory to cache */
void cache_insert(block_sector_t sector_idx, void *data) {
    lock_acquire(&cache_lock);
    /* Checks if the cache has already hit maximum capacity */
    if (hash_size(&cache_sector_table) == MAX_BUFFER_SIZE)
        cache_evict();
    lock_release(&cache_lock);

    ASSERT(hash_size(&cache_sector_table) < MAX_BUFFER_SIZE);
    cache_init(sector_idx, data);
}

/* Will retrieve the specific cache from the cache map. */
struct cache_sector *cache_get(block_sector_t sector_idx) {
    lock_acquire(&cache_lock);
    struct cache_sector temp_sector;
    struct cache_sector *return_sector = NULL;
    temp_sector.sector_idx = sector_idx;

    struct hash_elem *cache_sector_entry = hash_find(&cache_sector_table, &temp_sector.cache_sector_elem);

    if (cache_sector_entry) {
        return_sector = hash_entry(cache_sector_entry, struct cache_sector, cache_sector_elem);
    }
    lock_release(&cache_lock);
    return return_sector;
}

/* Will write data to a cache_sector buffer */
void write_to_cache(block_sector_t sector_idx, void *data) {
    struct cache_sector *found_sector = cache_get(sector_idx);

    /* We want to be sure that the sector we find is not null */
    ASSERT(found_sector != NULL);
    memcpy(found_sector->sector, data, BLOCK_SECTOR_SIZE);

    found_sector->is_dirty = true;
}


/* Will read from cache and write to memory. Involves freeing. */
void read_from_cache(block_sector_t sector_idx, void *data) {
    struct cache_sector *found_sector = cache_get(sector_idx);

    ASSERT(found_sector != NULL);
    memcpy(data, found_sector->sector, BLOCK_SECTOR_SIZE);
}
