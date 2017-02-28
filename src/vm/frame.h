/*! \file frame.h
 *
 * Declarations for the frame table
 */

#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>

/*! Entry for frame table. */
struct frame_table_entry {
    void *frame;                /*!< Address of frame. */
    struct thread *owner;       /*!< Process that is using the frame. */
    int pin_count;              /*!< Should not evict pinned pages. */
    struct list_elem frame_table_elem; /*!< Use list for LRU. */
};

void frame_table_init(void);
struct frame_table_entry *get_frame(void);
void free_frame(struct frame_table_entry *fte);

void pin(struct frame_table_entry *fte);
void unpin(struct frame_table_entry *fte);

#endif /* vm/frame.h */
