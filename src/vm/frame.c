#include "vm/frame.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "vm/page.h"
#include "vm/swap.h"

static struct list frame_table;

static struct lock frame_lock;

/* Acquire frame lock. */
void acquire_frame_lock(void) {
    lock_acquire(&frame_lock);
}

/* Release frame lock. */
void release_frame_lock(void) {
    lock_release(&frame_lock);
}
void frame_table_init(void) {
    list_init(&frame_table);
}

/*! Create a new frame table entry. */
static void *fte_create(void *frame, struct thread *owner) {
    struct frame_table_entry *fte;

    fte = palloc_get_page(PAL_USER | PAL_ZERO);
    while (fte == NULL) {
        evict();
        fte = palloc_get_page(PAL_USER | PAL_ZERO);
    }
    fte->frame = frame;
    fte->owner = owner;
    fte->pin_count = 0;
    fte->upage = NULL;

    return fte;
}

/*! Create new frame and frame table entry. */
struct frame_table_entry *get_frame(void) {
    /* Allocate page frame*/
    void *frame = palloc_get_page(PAL_USER | PAL_ZERO);
    while (frame == NULL) {
        evict();
        frame = palloc_get_page(PAL_USER | PAL_ZERO);
    }

    /* Obtain unused frame */
    struct frame_table_entry *fte = fte_create(frame, thread_current());

    /* Push frame on back of list */
    list_push_back(&frame_table, &fte->frame_table_elem);

    pin(fte);
    return fte;
}

/*! Choose a frame entry to be evicted. */
struct frame_table_entry *choose_frame_to_evict(void) {
    /* TODO: Implement more advanced algorithm. */
    struct list_elem *e = list_begin(&frame_table);
    struct frame_table_entry *fte =
        list_entry(e, struct frame_table_entry, frame_table_elem);
    return fte;
}

/*! Wrapper to choose a frame and evict it. */
void evict(void) {
    struct frame_table_entry *fte_to_evict = choose_frame_to_evict();
    evict_frame(fte_to_evict);
}

/*! Evict the specified frame. */
void evict_frame(struct frame_table_entry *fte) {
    acquire_frame_lock();
    ASSERT(fte->pin_count == 0);
    struct thread *owner = fte->owner;
    struct sup_page *page = thread_sup_page_get(&owner->sup_page, fte->upage);
    ASSERT(page != NULL);

    /* If possible, write to file */
    if (page->status == FILE_PAGE) {
        /* If dirty, write to file */
        if (sup_page_is_dirty(&owner->sup_page, fte->upage)) {
            struct file *file = page->file_stats->file;
            ASSERT(file != NULL);
            off_t offset = page->file_stats->offset;
            off_t read_bytes = page->file_stats->read_bytes;

            acquire_file_lock();
            file_write_at(file, page->addr, read_bytes, offset);
            release_file_lock();
        }
        /* If not dirty, it's already in file */
    }

    /* Otherwise, write to swap */
    else if (page->status == SWAP_PAGE) {
        /* Write to swap */
        acquire_swap_lock();
        page->swap_position = swap_table_out(page);
        release_swap_lock();
    }

    /* If ZERO_PAGE, no need to save */

    /* Update supplemental page table and virtual page*/
    pagedir_clear_page(owner->pagedir, page->addr);
    pagedir_set_accessed(owner->pagedir, page->addr, false);
    pagedir_set_dirty(owner->pagedir, page->addr, false);
    page->fte = NULL;

    /* Remove from frame table */
    list_remove(&fte->frame_table_elem);

    /* Free memory. */
    free_frame(fte);

    release_frame_lock();
}

/*! Free memory after safety checks. */
void free_frame(struct frame_table_entry *fte) {
    fte->upage = NULL;
    /* TODO: Might want to remove. */
    try_remove(&fte->frame_table_elem);
    /* Safety checks. */
    /* Check if list_elem was removed. */
    ASSERT(try_remove(&fte->frame_table_elem) == NULL);
    ASSERT(fte->pin_count == 0); /* Should be unpinned. */

    palloc_free_page(fte->frame);
    palloc_free_page(fte);
    fte = NULL;
}

/*! Pin frame so it isn't swapped before use. */
void pin(struct frame_table_entry *fte) {
    fte->pin_count++;
}
/*! Unpin to indicate that frame can be freed. */
void unpin(struct frame_table_entry *fte) {
    ASSERT(fte->pin_count > 0);
    fte->pin_count--;
}
