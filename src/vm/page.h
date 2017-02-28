/*! \file page.h
 *
 * Declarations for the supplemental page table
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <stdbool.h>
#include "vm/frame.h"

static struct hash sup_page_table;

enum page_status {
    SWAP_PAGE,
    FILE_PAGE,
    ZERO_PAGE
};

/*! Pages for supplemental page table. */
struct sup_page {
    void *addr;                           /*!< Address to the virtual page. */
    enum page_status status;              /*!< Current status of page. */
    int page_no;                          /*!< Page number. */
    struct hash_elem sup_page_table_elem; /*!< Elem for supplemental page table. */
};

/* Initializes supplemental page hash table */
void sup_page_table_init(void);
void fetch_data_to_frame(struct sup_page *page, struct frame_table_entry *fte);

struct sup_page *sup_page_get(void *addr);
unsigned sup_page_hash(const struct hash_elem *e, void *aux);
bool sup_page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux);
void sup_page_delete(void *addr);


#endif /* vm/page.h */
