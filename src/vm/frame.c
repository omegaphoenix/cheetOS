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

/* Frame table. */
static struct list frame_table;
/* Points to list_elem that is to be checked for eviction. */
static struct list_elem *clock_hand;

/* Lock for frame table. */
static struct lock frame_lock;
/* Lock for eviction. */
static struct lock eviction_lock;

/* Eviction and helper methods. */
static void evict_frame(struct frame_table_entry *fte);
static void increment_clock_hand(void);
static struct frame_table_entry *choose_frame_to_evict(void);
static void evict(void);

static void *fte_create(void *frame, struct thread *owner);

/*! Acquire frame lock. */
void acquire_frame_lock(void) {
    lock_acquire(&frame_lock);
}

/*! Release frame lock. */
void release_frame_lock(void) {
    lock_release(&frame_lock);
}

/* Acquire eviction lock. */
void acquire_eviction_lock(void) {
    lock_acquire(&eviction_lock);
}

/* Release eviction lock. */
void release_eviction_lock(void) {
    lock_release(&eviction_lock);
}

void frame_table_init(void) {
    clock_hand = NULL;
    list_init(&frame_table);
    lock_init(&frame_lock);
    lock_init(&eviction_lock);
}

/*! Create a new frame table entry. */
static void *fte_create(void *frame, struct thread *owner) {
    struct frame_table_entry *fte;

    fte = malloc(sizeof(struct frame_table_entry));
    if (fte == NULL) {
        PANIC("Not enough memory to create frame_table_entry!");
    }
    fte->pin_count = 0;
    pin(fte);
    fte->frame = frame;
    fte->owner = owner;
    fte->pagedir = NULL;
    fte->spte = NULL;
    fte->addr = NULL;
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
    struct thread *cur = thread_current();
    struct frame_table_entry *fte = fte_create(frame, cur);

    if (clock_hand == NULL) {
        /* Push frame on back of list. */
        acquire_frame_lock();
        list_push_back(&frame_table, &fte->frame_table_elem);
        release_frame_lock();
    }
    else {
        /* Insert before the clock_hand. */
        acquire_frame_lock();
        list_insert(clock_hand, &fte->frame_table_elem);
        release_frame_lock();

    }
    return fte;
}

/*! Move clock hand (next element to evict) to the next frame in the
    list. If we reach the end of the list, wrap around to the beginning. */
static void increment_clock_hand(void) {
    acquire_frame_lock();
    if (clock_hand == NULL || clock_hand == list_back(&frame_table)) {
        clock_hand = list_begin(&frame_table);
    }
    else {
        clock_hand = list_next(clock_hand);
    }
    if (list_size(&frame_table) == 1) {
        clock_hand = NULL;
    }
    release_frame_lock();
}

/*! Choose a frame entry to be evicted based on clock algorithm. */
static struct frame_table_entry *choose_frame_to_evict(void) {
    ASSERT(!list_empty(&frame_table));
    if (clock_hand == NULL) {
        increment_clock_hand();
    }
    acquire_frame_lock();
    struct frame_table_entry *fte =
        list_entry(clock_hand, struct frame_table_entry, frame_table_elem);
    release_frame_lock();
    struct sup_page *page = fte->spte;

    while (!is_user_vaddr(page->addr)
            || pagedir_is_accessed(fte->pagedir, fte->addr)
            || fte->pin_count > 0) {

        if (fte->pin_count == 0 && is_user_vaddr(page->addr)) {
            pagedir_set_accessed(fte->pagedir, fte->addr, false);
        }

        increment_clock_hand();
        acquire_frame_lock();
        fte = list_entry(clock_hand, struct frame_table_entry,
                frame_table_elem);
        release_frame_lock();

        page = fte->spte;
    }

    increment_clock_hand();
    return fte;
}

/*! Wrapper to choose a frame and evict it. */
static void evict(void) {
    acquire_eviction_lock();
    struct frame_table_entry *fte_to_evict = choose_frame_to_evict();
    evict_frame(fte_to_evict);

    /* Free memory. */
    free_frame(fte_to_evict);
    release_eviction_lock();
}

/*! Wrapper to evict frame. */
void evict_chosen_frame(struct frame_table_entry *fte, bool locked) {
    if (!locked) {
        acquire_eviction_lock();
    }
    else {
        ASSERT(lock_held_by_current_thread(&eviction_lock));
    }

    /* Make sure clock hand isn't pointing to evicted frame. */
    if (&fte->frame_table_elem == clock_hand) {
        if (list_size(&frame_table) > 1) {
            increment_clock_hand();
        }
        else {
            clock_hand = NULL;
        }
    }

    evict_frame(fte);
    free_frame(fte);

    if (!locked) {
        release_eviction_lock();
    }
}

/*! Evict the specified frame. */
static void evict_frame(struct frame_table_entry *fte) {
    ASSERT(fte->pin_count == 0);
    struct sup_page *page = fte->spte;
    ASSERT(page != NULL);
    ASSERT(is_user_vaddr(page->addr));

    /* We only evict dirty stuff */
    if (pagedir_is_dirty(fte->pagedir, fte->addr) || page->status == SWAP_PAGE) {
        /* If mmapped, write to file */
        if (page->is_mmap) {
            pin(fte);
            /* If dirty, maybe write */
            struct file *file = page->file_stats->file;
            ASSERT(file != NULL);
            off_t offset = page->file_stats->offset;
            acquire_file_lock();
            file_write_at(file, page->addr, PGSIZE, offset);
            release_file_lock();

            unpin(fte);
        }
        /* Otherwise, write to swap */
        else {
            /* Write to swap */
            page->swap_position = swap_table_out(page);
        }
    }
    /* Page is no longer loaded */
    page->loaded = false;

    /* Update supplemental page table and virtual page. */
    if (fte->pagedir != NULL) {
        pagedir_clear_page(fte->pagedir, fte->addr);
    }

    page->fte = NULL;
}

/*! Free memory after safety checks. */
void free_frame(struct frame_table_entry *fte) {
    fte->spte = NULL;
    /* Remove from frame table */
    acquire_frame_lock();
    try_remove(&fte->frame_table_elem);
    release_frame_lock();

    /* Safety checks. */
    /* Check if list_elem was removed. */
    acquire_frame_lock();
    ASSERT(try_remove(&fte->frame_table_elem) == NULL);
    release_frame_lock();
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
