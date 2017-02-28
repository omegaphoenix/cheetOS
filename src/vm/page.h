/*! \file page.h
 *
 * Declarations for the supplemental page table
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>

static struct hash sup_page_table;

enum page_status {
    SWAP,
    FILESYS,
    ZERO
}

/*! Pages for supplemental page table. */
struct sup_page {
    page_status status;             /*!< Current status of page. */
    int page_no;                    /*!< Page number. */
    hash_elem sup_page_table_elem;  /*!< Elem for supplemental page table. */
};

#endif /* vm/page.h */
