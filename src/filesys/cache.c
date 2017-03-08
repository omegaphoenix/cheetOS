#include <hash.h>
#include <stdbool.h>

#include "cache.h"
#include "devices/block.h"
#include "threads/synch.h"
#include "threads/malloc.h"

static unsigned hash_sector_idx(const struct hash_elem *e, void *aux);
static bool hash_sector_less(const struct hash_elem *a, const struct hash_elem *b, void *aux);
static void hash_cache_free(struct hash_elem *e, void *aux);

/* Will initialize the global hash table. */
void cache_table_init(void) {
    hash_init(&cache_sector_table, hash_sector_idx, hash_sector_less);
    lock_init(&cache_lock);
}

/* Frees elements after they are removed from table */
void cache_table_free(void) {
    hash_destroy(&cache_sector_table, hash_cache_free);
}

/* Hashes a cache_sector into the hash table based on its sector idx */
static unsigned hash_sector_idx(const struct hash_elem *e, void *aux UNUSED) {
    struct cache_sector *sector = hash_entry(e, struct cache_sector, cache_sector_elem);
    int hash_idx = hash_int(sector->sector_idx);

    return hash_idx;
}

/* Compares the sector indices for hash element comparisons */
static unsigned hash_sector_less(const struct hash_elem *a, const struct hash_elem *b, void *aux) {
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
struct cache_sector *cache_init(block_sector_t sector_idx) {
    /* First, check that it doesn't exist */
    ASSERT(cache_get(sector_idx) == NULL);
    int byte;
    struct cache_sector *new_sector = malloc(sizeof(struct cache_sector));

    new_sector->sector_idx = sector_idx;
    new_sector->access_level = NOT_ACCESSED;
    new_sector->is_dirty = false;

    /* Make the sector empty. */
    for (byte = 0; byte < BLOCK_SECTOR_SIZE; byte++) {
        new_sector->sector[byte] = 0;
    }

    hash_insert(&cache_sector_table, &new_sector->cache_sector_elem);

    return new_sector;
}

/* Finds appropriate struct from hash table, and frees it/deletes it from table */
void cache_free(block_sector_t sector_idx) {
    struct cache_sector temp_sector;
    temp_sector.sector_idx = sector_idx;

    struct hash_elem *cache_sector_entry = hash_find(&cache_sector_table, &temp_sector.cache_sector_elem);

    if (cache_sector_entry) {
        cache_sector_entry = hash_delete(&cache_sector_table, cache_sector_entry);

        ASSERT(cache_sector_entry != NULL);
        ASSERT(cache_get(sector_idx) == NULL);

        hash_cache_free(cache_sector_entry);
    }
}

/* Uses block algorithm to choose the next block to evict */
void cache_evict(void) {
  // TODO
}

/* Removes a block from the buffer cache. This involves it writing from cache back
   to memory */
void cache_remove(block_Sector_t sector_idx) {
  // TODO
}

/* Adds a block into buffer cache. Evicts if necessary. Will write from
   memory to cache */
void cache_insert(block_sector_t, sector_idx, void *data) {
  // TODO
}

/* Will retrieve the specific cache from the cache map. */
struct cache_sector *cache_get(block_sector_t sector_idx) {
    struct cache_sector temp_sector;
    struct cache_sector *return_sector = NULL;
    temp_sector.sector_idx = sector_idx;

    struct hash_elem *cache_sector_entry = hash_find(&cache_sector_table, &temp_sector.cache_sector_elem);

    if (cache_sector_entry) {
        return_sector = hash_entry(cache_sector_entry, struct cache_sector, cache_sector_elem);
    }
    return return_sector;
}

/* Will write data to a cache_sector buffer */
void write_to_cache(block_sector_t sector_idx, void *data) {
  // TODO
}

/* Will read from cache and write to memory. Involves freeing. */
void read_from_cache(block_sector_t sector_idx, void *data) {
  // TODO
}
