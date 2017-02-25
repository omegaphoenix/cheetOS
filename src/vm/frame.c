#include "vm/frame.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"

static struct list frame_table;

void frame_table_init(void) {
    list_init(&frame_table);
}

/*! Create a new frame table entry. */
static void *fte_create(void *frame, struct thread *owner) {
    struct frame_table_entry *fte;

    fte = palloc_get_page(PAL_USER | PAL_ZERO); // but do we need a whole page for this...
    if (fte == NULL) {
        /* TODO: evict frame */
        PANIC("store_frame: out of frames");
    }
    fte->frame = frame;
    fte->owner = owner;
    fte->pin_count = 0;

    return fte;
}

/*! Create new frame and frame table entry. */
void *get_frame(void) {
    /* Allocate page frame*/
    void *frame = palloc_get_page(PAL_USER | PAL_ZERO);
    if (frame == NULL) {
        /* TODO: evict frame */
        PANIC("store_frame: out of pages");
    }

    /* Obtain unused frame */
    struct frame_table_entry *fte = fte_create(frame, thread_current());

    /* Push frame on back of list */
    list_push_back(&frame_table, &fte->frame_table_elem);

    return fte;
}

/*! Free memory after safety checks. */
void free_frame(struct frame_table_entry *fte) {
    /* Safety checks. */
    /* Check if list_elem was removed. */
    ASSERT(list_remove(&fte->frame_table_elem) == NULL);
    ASSERT(fte->pin_count == 0); /* Should be unpinned. */

    palloc_free_page(fte->frame);
    palloc_free_page(fte);
}