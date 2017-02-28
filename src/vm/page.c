#include "vm/page.h"
#include <hash.h>

/*! Initialize supplemental page table. */
void sup_page_table_init(void) {
    /* TODO */
    /* Initialize hash table. */
}

/*! Return supplemental page from hash table. */
struct sup_page *get_sup_page(void *addr) {
    /* TODO */
    return NULL;
}

/*! Copy data to the frame table. */
void fetch_data_to_frame(struct sup_page *page,
        struct frame_table_elem *fte) {
}
