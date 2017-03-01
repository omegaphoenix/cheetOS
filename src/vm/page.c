#include "vm/page.h"
#include <debug.h>
#include <hash.h>
#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "vm/frame.h"

static void get_swap_page(struct sup_page *page,
        struct frame_table_entry *fte);
static void get_file_page(struct sup_page *page,
        struct frame_table_entry *fte);
static void get_zero_page(struct sup_page *page,
        struct frame_table_entry *fte);

/*! Initialize supplemental page table. */
struct hash * sup_page_table_init(void) {
    /* For this simple hash table, no auxiliary data should be necessary */
    struct hash * thread_sup_table = malloc(sizeof(struct hash));
    hash_init(thread_sup_table, sup_page_hash, sup_page_less, NULL);
    return thread_sup_table;
}

/* Frees a hash table */
void sup_page_table_delete(struct hash * hash_table) {
    /* TODO: Free everything inside hash table if not freed */
    free(hash_table);
}

/*! Since we're hashing by address, we will be using hash_bytes
    supplied by Pintos. */
unsigned sup_page_hash(const struct hash_elem *e, void *aux UNUSED) {
    const struct sup_page *page = hash_entry(e, struct sup_page, sup_page_table_elem);
    int hash_index = hash_bytes(page->addr, sizeof(page->addr));

    return hash_index;
}

/* Hash table's comparison will be using pointer comparisons */
bool sup_page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
    const struct sup_page *first_page = hash_entry(a, struct sup_page, sup_page_table_elem);
    const struct sup_page *second_page = hash_entry(b, struct sup_page, sup_page_table_elem);

    return first_page->addr < second_page->addr;
}

/* Delete an entry from hash table using the address of the page */
void sup_page_delete(struct hash * hash_table, void *addr) {
    struct sup_page temp_page;
    struct sup_page *page_to_delete = NULL;
    struct hash_elem *elem_to_delete = NULL;
    struct hash_elem *deleted_elem = NULL;

    temp_page.addr = addr;

    elem_to_delete = hash_find(hash_table, &temp_page.sup_page_table_elem);

    /* Found the element inside the hash table.*/
    if (elem_to_delete != NULL) {
        /* Deleted elem must exist because elem_to_delete is inside hash table */
        deleted_elem = hash_delete(hash_table, elem_to_delete);

        /* Defensively check to see if it's still inside the hash_table/deleted_elem is not null */
        ASSERT(deleted_elem != NULL);
        ASSERT(sup_page_get(hash_table, addr) == NULL);

        /* Retrieve sup_page struct */
        page_to_delete = hash_entry(deleted_elem, struct sup_page, sup_page_table_elem);

        /* TODO: free deleted_elem stuff before freeing entire struct */
        palloc_free_page(page_to_delete);
        page_to_delete = NULL;
    }
}

/* Retrieves a supplemental page from the hash table via address */
struct sup_page *sup_page_get(struct hash * hash_table, void *addr) {
    struct sup_page temp_page;
    struct sup_page *return_page = NULL;
    struct hash_elem *temp_elem = NULL;
    temp_page.addr = addr;

    /* Recall that temp_elem is a hash_elem, so we need to hash_entry it */
    temp_elem = hash_find(hash_table, &temp_page.sup_page_table_elem);

    if (temp_elem != NULL) {
      return_page = hash_entry(temp_elem, struct sup_page, sup_page_table_elem);
    }
    return return_page;
}

void sup_page_insert(struct hash * hash_table, void *addr) {

}

/*! Copy data to the frame table. */
void fetch_data_to_frame(struct sup_page *page,
        struct frame_table_entry *fte) {
    switch (page->status) {
        case SWAP_PAGE:
            get_swap_page(page, fte);
            break;
        case FILE_PAGE:
            get_file_page(page, fte);
            break;
        case ZERO_PAGE:
            get_zero_page(page, fte);
            break;
    }
}

static void get_swap_page(struct sup_page *page,
        struct frame_table_entry *fte) {
}

static void get_file_page(struct sup_page *page,
        struct frame_table_entry *fte) {
}

static void get_zero_page(struct sup_page *page,
        struct frame_table_entry *fte) {
    memset(fte->frame, 0, PGSIZE);
}
