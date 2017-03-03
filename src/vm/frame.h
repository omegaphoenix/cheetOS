/*! \file frame.h
 *
 * Declarations for the frame table
 */

#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>

/*! Entry for frame table. */
struct frame_table_entry {
    void *frame;                /*!< Address of frame (kernel virtual address). */
    void *upage;                /*!< User virtual address. */
    struct thread *owner;       /*!< Process that is using the frame. */
    int pin_count;              /*!< Should not evict pinned pages. */
    struct list_elem frame_table_elem; /*!< Use list for LRU. */
};

void frame_table_init(void);
struct frame_table_entry *get_frame(void);

struct frame_table_entry *choose_frame_to_evict(void);
void evict(void);
void evict_frame(struct frame_table_entry *fte);

void free_frame(struct frame_table_entry *fte);

void pin(struct frame_table_entry *fte);
void unpin(struct frame_table_entry *fte);

void acquire_frame_lock(void);
void release_frame_lock(void);

#endif /* vm/frame.h */
