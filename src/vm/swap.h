/*! \file swap.h
 *
 * Declarations for the swap partition
 */

#ifndef SWAP_H
#define SWAP_H

#include <bitmap.h>
#include <stdbool.h>
#include "devices/block.h"
#include "threads/vaddr.h"
#include "vm/page.h"
#include "vm/frame.h"

/* BLOCK_SECTOR_SIZE is less than PGSIZE, so we want to see by what factor */
#define SECTORS_PER_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)

/* We're using bitmap, so we define a bit for occupied and a bit for not occupied */
#define SWAP_EMPTY 0
#define SWAP_OCCUPIED 1
#define SINGLE_BIT 1
#define SWAP_BITMAP_START 0

struct swap_table {
    struct block *swap_block;
    struct bitmap *swap_bitmap;
};

/* Function definitions */

void swap_table_init(void);
void swap_table_free(void);
size_t swap_table_out(struct sup_page *evicted_page);
bool swap_table_in(struct sup_page *dest_page, struct frame_table_entry *fte);
void acquire_swap_lock(void);
void release_swap_lock(void);

#endif /* SWAP_H */