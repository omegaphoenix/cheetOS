/*! \file page.h
 *
 * Declarations for the supplemental page table
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <stdbool.h>
#include "filesys/off_t.h"
#include "vm/frame.h"
#include "threads/thread.h"

enum page_status {
    SWAP_PAGE,
    FILE_PAGE,
    ZERO_PAGE
};

struct segment {
    struct file *file;                    /*!< File struct */
    uint32_t read_bytes;                  /*!< How many bytes need to be read */
    uint32_t zero_bytes;                  /*!< How many bytes are zeroed */
    off_t ofs;                            /*!< File offset */
    bool writable;                       /*!< Whether page is writeable. */
};

/*! Pages for supplemental page table. */
struct sup_page {
    enum page_status status;              /*!< Current status of page. */
    struct hash_elem sup_page_table_elem; /*!< Elem for supplemental page table. */
    uint8_t *upage;                       /*!< Location of the virtual page */
    struct segment *segment_info;         /*!< Information on segment */
};

/* Initializes supplemental page hash table */
void thread_sup_page_table_init(struct thread *t);
void thread_sup_page_table_delete(struct thread *t);
void fetch_data_to_frame(struct sup_page *page, struct frame_table_entry *fte);

struct sup_page *thread_sup_page_get(struct hash * hash_table, void *addr);
unsigned sup_page_hash(const struct hash_elem *e, void *aux);
bool sup_page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux);
void sup_page_delete(struct hash * hash_table, void *addr);
struct hash_elem * sup_page_insert(struct hash * hash_table, struct sup_page * new_page);


#endif /* vm/page.h */
