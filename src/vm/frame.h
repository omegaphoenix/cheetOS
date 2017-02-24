/*! \file frame.h
 *
 * Declarations for the frame table
 */

#ifndef VM_FRAME_H
#define VM_FRAME_H

struct frame_table_entry {
    void *frame;
    struct thread *owner;       /* process that is using the frame */
    int pin_count;
    struct list_elem frame_table_elem;
};

void *store_frame(void);

#endif /* vm/frame.h */
