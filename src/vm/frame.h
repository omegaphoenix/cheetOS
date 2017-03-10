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
    void *addr;                 /*!< Address of page (user virtual address). */
    uint32_t *pagedir;          /*!< Page directory. */
    struct sup_page *spte;      /*!< Supplementary Page Table */
    struct thread *owner;       /*!< Process that is using the frame. */
    int pin_count;              /*!< Should not evict pinned pages. */
    struct list_elem frame_table_elem; /*!< Use list for Clock. */
};

/* Handle locks. */
void acquire_frame_lock(void);
void release_frame_lock(void);
void acquire_eviction_lock(void);
void release_eviction_lock(void);

void frame_table_init(void);
struct frame_table_entry *get_frame(void);

void evict_chosen_frame(struct frame_table_entry *fte);
void free_frame(struct frame_table_entry *fte);

void pin(struct frame_table_entry *fte);
void unpin(struct frame_table_entry *fte);

#endif /* vm/frame.h */
