/*! \file cache.h
 *
 * Declarations for the buffer cache data structure
 */
#ifndef BUFFER_CACHE_H_
#define BUFFER_CACHE_H_

#include <hash.h>
#include <stdbool.h>
#include "devices/block.h"
#include "threads/synch.h"

#define MAX_BUFFER_SIZE 64

struct hash cache_sector_table;

/* We would like our cache sector to be in a hash table for
   easier lookup. We will use sector index as the key. */
struct cache_sector {
    block_sector_t sector_idx;          /*!< Sector index on filesys block. */
    bool accessed;                      /*!< Sector access level. */
    bool dirty;                         /*!< Boolean if sector is dirty. */
    struct hash_elem cache_hash_elem;   /*!< Makes it part of a hash table. */
    struct list_elem cache_list_elem;   /*!< Makes it part of a hash table. */
    uint8_t sector[BLOCK_SECTOR_SIZE];  /*!< Each sector is 512 bytes. */
};

/* Hash table constructor/destructor. */
void cache_table_init(void);
void cache_table_free(void);

/* cache_sector constructor/destructor. Removal/insertion into hash table. */
struct cache_sector *cache_init(block_sector_t sector_idx, void *data);
void cache_free(block_sector_t sector_idx);

/* Might use clock algorithm for this. */
void cache_evict(void);

/* Insertion from buffer cache. */
void cache_insert(block_sector_t sector_idx, void *data);

/* Retrieval. */
struct cache_sector *cache_get(block_sector_t sector_idx);

/* Writing/Reading to/from disk methods. */
void write_to_cache(block_sector_t sector_idx, void *data);
void read_from_cache(block_sector_t sector_idx, void *data);

#endif /* BUFFER_CACHE_H_ */
