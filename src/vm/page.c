#include "vm/page.h"
#include <hash.h>

/*! Initialize supplemental page table. */
void sup_page_table_init(void) {
    /* For this simple hash table, no auxiliary data should be necessary */
    hash_init(&sup_page_table, page_hash, page_less, NULL);
}

/*! Since we're hashing by address, we will be using hash_bytes
    supplied by Pintos. */
unsigned page_hash(const struct hash_elem *e, void *aux UNUSED) {
    const struct sup_page *page = hash_entry(e, struct sup_page, struct hash_elem);
    int hash = hash_bytes(page->addr, sizeof(page->addr));

    return hash;
}

bool page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
    const struct sup_page *first_page = hash_entry(a, struct sup_page, struct hash_elem);
    const struct sup_page *second_page = hash_entry(b, struct sup_page, struct hash_elem);

    return first_page->addr < second_page->addr;
}

/*! Return supplemental page from hash table. */
struct sup_page *get_sup_page(void *addr) {
    struct sup_page temp_page;
    struct sup_page *return_page = NULL;
    struct hash_elem *temp_elem;
    temp_page.addr = addr;

    /* Recall that temp_elem is a hash_elem, so we need to hash_entry it */
    temp_elem = hash_find(&sup_page_table, &temp_page.hash_elem);

    if (temp_elem != NULL) {
      return_page = hash_entry(temp_elem, struct sup_page, struct hash_elem);
    }
    return return_page;
}

/*! Copy data to the frame table. */
void fetch_data_to_frame(struct sup_page *page,
        struct frame_table_elem *fte) {
}
