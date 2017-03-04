#include <debug.h>

#include "vm/swap.h"
#include "vm/page.h"
#include "threads/synch.h"
#include "userprog/process.h"

static struct swap_table global_swap;

static struct lock swap_lock;

/* Acquire's the swap lock so that there isn't concurrent writing to swap */
void acquire_swap_lock(void) {
    lock_acquire(&swap_lock);
}

/* Release the swap's lock to allow others to write into it */
void release_swap_lock(void) {
    lock_release(&swap_lock);
}

/* Initialize the swap table. Populating the blocks and
   the bitmap. */
void swap_table_init(void) {
    lock_init(&swap_lock);
    /* Based on the number of slots, we want that number of bits */
    global_swap.swap_block = block_get_role(BLOCK_SWAP);
    int size = block_size(global_swap.swap_block);

    /* Each swap slot is PGSIZE, but we only have block_sector_sizes.
       It takes SECTORS_PER_PAGE sector sizes to hit 1 page memory.
       So we divide size by sectors_per_page to find out how many
       groups of 1 page slots we have (recall that size is in terms
       of block_sector_sizes) */
    global_swap.swap_bitmap = bitmap_create(size / SECTORS_PER_PAGE);

    /* Check if allocations worked. */
    if (global_swap.swap_bitmap == NULL || global_swap.swap_block == NULL) {
        PANIC("Swap partition not allocated properly!");
    }

    /* Set all bits to EMPTY at first */
    bitmap_set_all(global_swap.swap_bitmap, SWAP_EMPTY);
}

/* Freeing the swap table... */
void swap_table_free(void) {
    bitmap_destroy(global_swap.swap_bitmap);
}

/* This will take in an entry of a page table,
   and insert it into the swap table */
size_t swap_table_out(struct sup_page *evicted_page) {
    acquire_swap_lock();
    size_t swap_idx;
    int cnt_sector;
    evicted_page->status = SWAP_PAGE;
    struct frame_table_entry *fte = evicted_page->fte;

    /* Start using physical address */
    uint8_t *kpage = (uint8_t *) fte->frame;


    swap_idx = bitmap_scan_and_flip(global_swap.swap_bitmap,
                                    SWAP_BITMAP_START,
                                    SINGLE_BIT,
                                    SWAP_EMPTY);

    if (swap_idx == BITMAP_ERROR) {
        release_swap_lock();
        PANIC("Swap is full!");
    }

    /* Read into each sector of a swap slot at idx */
    /* Recall that this writes in BLOCK_SECTOR_SIZE amounts at a time */
    for (cnt_sector = 0; cnt_sector < SECTORS_PER_PAGE; cnt_sector++) {
        int block_offset = swap_idx * SECTORS_PER_PAGE + cnt_sector;
        block_write(global_swap.swap_block,
                    block_offset,
                    kpage + cnt_sector * BLOCK_SECTOR_SIZE);
    }
    release_swap_lock();

    return swap_idx;
}

/* This will take a swap index, and then write it into a frame.
   It will also free up a space in the swap table. */
bool swap_table_in(struct sup_page *dest_page, struct frame_table_entry *fte) {

    acquire_swap_lock();
    int swap_idx = dest_page->swap_position;
    ASSERT(swap_idx > -1);

    if (bitmap_test(global_swap.swap_bitmap, swap_idx) != SWAP_OCCUPIED) {
        release_swap_lock();
        return false;
    }

    int cnt_sector;

    uint8_t *kpage = (uint8_t *) fte->frame;
    uint8_t *upage = (uint8_t *) dest_page->addr;

    if (!install_page(upage, kpage, dest_page->writable)) {
        free_frame(fte);
        release_swap_lock();
        return false;
    }


    /* Free the slot */
    bitmap_flip(global_swap.swap_bitmap, swap_idx);

    /* Read into each frame buffer from a swap slot at idx */
    /* Recall that this writes in BLOCK_SECTOR_SIZE amounts at a time */
    for (cnt_sector = 0; cnt_sector < SECTORS_PER_PAGE; cnt_sector++) {
        int block_offset = swap_idx * SECTORS_PER_PAGE + cnt_sector;
        block_read(global_swap.swap_block,
                    block_offset,
                    kpage + cnt_sector * BLOCK_SECTOR_SIZE);
    }

    release_swap_lock();
    return true;
}

