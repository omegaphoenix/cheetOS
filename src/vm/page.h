/*! \file page.h
 *
 * Declarations for the supplemental page table
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>

static struct hash sup_page_table;

enum page_status {
    SWAP_PAGE,
    FILE_PAGE,
    ZERO_PAGE
};

/*! Pages for supplemental page table. */
struct sup_page {
    enum page_status status;              /*!< Current status of page. */
    int page_no;                          /*!< Page number. */
    struct hash_elem sup_page_table_elem; /*!< Elem for supplemental page table. */
};

void sup_page_table_init(void);
struct get_sup_page *get_sup_page(void *addr);

#endif /* vm/page.h */
