#include "vm/frame.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "vm/page.h"
#include "vm/swap.h"

static struct list frame_table;
/* Points to list_elem that is to be checked for eviction. */
static struct list_elem *clock_hand;

static struct lock frame_lock;

static void increment_clock_hand(void);

/* Acquire frame lock. */
void acquire_frame_lock(void) {
    lock_acquire(&frame_lock);
}

/* Release frame lock. */
void release_frame_lock(void) {
    lock_release(&frame_lock);
}
void frame_table_init(void) {
    clock_hand = NULL;
    list_init(&frame_table);
    lock_init(&frame_lock);
}

/*! Create a new frame table entry. */
static void *fte_create(void *frame, struct thread *owner) {
    struct frame_table_entry *fte;

    fte = malloc(sizeof(struct frame_table_entry));
    if (fte == NULL) {
        PANIC("Not enough memory to create frame_table_entry!");
    }
    fte->frame = frame;
    fte->owner = owner;
    fte->pin_count = 0;
    fte->spte = NULL;

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
    if (clock_hand == NULL) {
        list_push_back(&frame_table, &fte->frame_table_elem);
    }
    else {

        /* ERrors in here somehow. Something fails here and we call page fault. */
        list_insert(clock_hand, &fte->frame_table_elem);

    }
    pin(fte);
    return fte;
}

/*! Move clock hand (next element to evict) to the next frame in the
    list. If we reach the end of the list, wrap around to the beginning. */
static void increment_clock_hand(void) {
    if (clock_hand == NULL || clock_hand == list_end(&frame_table)) {
        clock_hand = list_begin(&frame_table);
    }
    else {
        clock_hand = list_next(clock_hand);
    }
}

/*! Choose a frame entry to be evicted based on clock algorithm. */
struct frame_table_entry *choose_frame_to_evict(void) {
    ASSERT(!list_empty(&frame_table));
    if (clock_hand == NULL) {
        increment_clock_hand();
    }
    struct frame_table_entry *fte =
        list_entry(clock_hand, struct frame_table_entry, frame_table_elem);
    struct thread *owner = fte->owner;
    struct sup_page *page = fte->spte;
    while (!is_user_vaddr(page->addr)
            || sup_page_is_accessed(owner, page->addr)
            || fte->pin_count > 0) {

        if (is_user_vaddr(page->addr))
            sup_page_set_accessed(fte->owner, page->addr, false);

        increment_clock_hand();
        fte = list_entry(clock_hand, struct frame_table_entry,
                frame_table_elem);

        page = fte->spte;
        owner = fte->owner;

    }
    increment_clock_hand();
    return fte;
}

/*! Wrapper to choose a frame and evict it. */
void evict(void) {
    acquire_frame_lock();
    struct frame_table_entry *fte_to_evict = choose_frame_to_evict();
    evict_frame(fte_to_evict);
    release_frame_lock();
}

/*! Evict the specified frame. */
void evict_frame(struct frame_table_entry *fte) {
    ASSERT(fte->pin_count == 0);
    struct thread *owner = fte->owner;
    struct sup_page *page = thread_sup_page_get(&owner->sup_page, fte->spte->addr);

    ASSERT(page != NULL);

    /* If mmapped, write to file */
    if (page->is_mmap) {
        /* If dirty, maybe write */
        if (sup_page_is_dirty(owner, page->addr)) {
            struct file *file = page->file_stats->file;
            ASSERT(file != NULL);
            off_t offset = page->file_stats->offset;

            acquire_file_lock();
            file_write_at(file, page->addr, PGSIZE, offset);
            release_file_lock();
        }
    }
    /* Otherwise, write to swap */
    else if (page->status == FILE_PAGE ||
            sup_page_is_dirty(owner, page->addr)) {
        /* Write to swap */
        page->swap_position = swap_table_out(page);
        page->status = SWAP_PAGE;
    }

    /* If ZERO_PAGE, no need to save */

    /* Update supplemental page table and virtual page*/
    pagedir_clear_page(owner->pagedir, page->addr);
    pagedir_set_accessed(owner->pagedir, page->addr, false);
    pagedir_set_dirty(owner->pagedir, page->addr, false);
    page->fte = NULL;

    /* Free memory. */
    free_frame(fte);
}

/*! Free memory after safety checks. */
void free_frame(struct frame_table_entry *fte) {
    fte->spte = NULL;
    /* Remove from frame table */
    list_remove(&fte->frame_table_elem);
    /* Safety checks. */
    /* Check if list_elem was removed. */
    ASSERT(try_remove(&fte->frame_table_elem) == NULL);
    ASSERT(fte->pin_count == 0); /* Should be unpinned. */

    palloc_free_page(fte->frame);
    free(fte);
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
