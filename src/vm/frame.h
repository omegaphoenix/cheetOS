/*! \file frame.h
 *
 * Declarations for the frame table
 */

#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>

struct frame_table_entry {
    void *frame;
    struct thread *owner;       /* process that is using the frame */
    int pin_count;
    struct list_elem frame_table_elem;
};

void frame_table_init(void);
void *get_frame(void);
void free_frame(struct frame_table_entry *fte);

#endif /* vm/frame.h */
