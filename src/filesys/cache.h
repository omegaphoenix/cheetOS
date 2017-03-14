/*! \file cache.h
 *
 * Declarations for the buffer cache data structure
 */
#ifndef BUFFER_CACHE_H_
#define BUFFER_CACHE_H_

#include <stdbool.h>
#include "devices/block.h"
#include "filesys/off_t.h"
#include "threads/synch.h"

#define MAX_BUFFER_SIZE 64

/* We would like our cache sector to be in a list for
   easier eviction. We will use sector index as the key. */
struct cache_sector {
    block_sector_t sector_idx;          /*!< Sector index on filesys block. */
    bool valid;                         /*!< True if sector is used. False when evicted. */
    bool accessed;                      /*!< Sector access level. */
    bool dirty;                         /*!< Boolean if sector is dirty. */
    struct list_elem cache_list_elem;   /*!< Makes it part of an eviction list. */
    uint8_t sector[BLOCK_SECTOR_SIZE];  /*!< Each sector is 512 bytes. */
    struct rw_lock read_write_lock;     /*!< For synchronizing readers/writers. */
    struct lock block_lock;             /*!< Lock for using block. */
    bool pin_count;                      /*!< Pin to prevent eviction. */
};

/* Cache initialization. */
void cache_table_init(void);

/* Writing/Reading to/from disk methods. */
void write_to_cache(block_sector_t sector_idx, const void *data);
void write_cache_offset(block_sector_t sector_idx, const void *data, off_t ofs,
    size_t bytes);
void read_from_cache(block_sector_t sector_idx, void *data);
void read_cache_offset(block_sector_t sector_idx, void *data, off_t ofs,
    size_t bytes);

#endif /* BUFFER_CACHE_H_ */
