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

    fte = palloc_get_page(PAL_USER | PAL_ZERO);
    if (fte == NULL) {
        /* TODO: evict frame */
        PANIC("store_frame: out of frames");
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
    if (frame == NULL) {
        /* TODO: evict frame */
        PANIC("store_frame: out of pages");
    }

    /* Obtain unused frame */
    struct frame_table_entry *fte = fte_create(frame, thread_current());

    /* Push frame on back of list */
    list_push_back(&frame_table, &fte->frame_table_elem);

    pin(fte);
    return fte;
}

void evict(struct frame_table_entry *fte) {
    ASSERT(fte->pin_count == 0);
    /* Write data. */

    /* Free memory. */
    free_frame(fte);
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
