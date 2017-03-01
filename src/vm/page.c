#include "vm/page.h"
#include <debug.h>
#include <hash.h>
#include <string.h>

#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "vm/frame.h"

static bool get_swap_page(struct sup_page *page);
static bool get_file_page(struct sup_page *page);
static bool get_zero_page(struct sup_page *page);

/*! Initialize supplemental page table. */
void thread_sup_page_table_init(struct thread *t) {
    /* For this simple hash table, no auxiliary data should be necessary */
    hash_init(&t->sup_page, sup_page_hash, sup_page_less, NULL);
}

/* Frees a hash table */
void thread_sup_page_table_delete(struct thread *t) {
    /* TODO: Free everything inside hash table if not freed */
    hash_destroy(&t->sup_page, NULL);
}

/*! Since we're hashing by address, we will be using hash_bytes
    supplied by Pintos. */
unsigned sup_page_hash(const struct hash_elem *e, void *aux UNUSED) {
    const struct sup_page *page = hash_entry(e, struct sup_page, sup_page_table_elem);
    int hash_index = hash_bytes(&page->upage, sizeof(page->upage));

    return hash_index;
}

/* Hash table's comparison will be using pointer comparisons */
bool sup_page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
    const struct sup_page *first_page = hash_entry(a, struct sup_page, sup_page_table_elem);
    const struct sup_page *second_page = hash_entry(b, struct sup_page, sup_page_table_elem);

    return first_page->upage < second_page->upage;
}

/* Delete an entry from hash table using the address of the page */
void sup_page_delete(struct hash * hash_table, void *addr) {
    struct sup_page temp_page;
    struct sup_page *page_to_delete = NULL;
    struct hash_elem *elem_to_delete = NULL;
    struct hash_elem *deleted_elem = NULL;

    temp_page.upage = addr;

    elem_to_delete = hash_find(hash_table, &temp_page.sup_page_table_elem);

    /* Found the element inside the hash table.*/
    if (elem_to_delete != NULL) {
        /* Deleted elem must exist because elem_to_delete is inside hash table */
        deleted_elem = hash_delete(hash_table, elem_to_delete);

        /* Defensively check to see if it's still inside the hash_table/deleted_elem is not null */
        ASSERT(deleted_elem != NULL);
        ASSERT(thread_sup_page_get(hash_table, addr) == NULL);

        /* Retrieve sup_page struct */
        page_to_delete = hash_entry(deleted_elem, struct sup_page, sup_page_table_elem);

        /* TODO: free deleted_elem stuff before freeing entire struct */
        palloc_free_page(page_to_delete);
        page_to_delete = NULL;
    }
}

/* Retrieves a supplemental page from the hash table via address */
struct sup_page *thread_sup_page_get(struct hash * hash_table, void *addr) {
    printf("Grabbing a page of address ... %p\n", (void *) addr);
    struct sup_page temp_page;
    struct sup_page *return_page = NULL;
    struct hash_elem *temp_elem = NULL;
    temp_page.upage = addr;

    /* Recall that temp_elem is a hash_elem, so we need to hash_entry it */
    temp_elem = hash_find(hash_table, &temp_page.sup_page_table_elem);

    if (temp_elem != NULL) {
      return_page = hash_entry(temp_elem, struct sup_page, sup_page_table_elem);
    }
    return return_page;
}

/* Inserts an address into a hash table */
struct hash_elem * sup_page_insert(struct hash * hash_table, struct sup_page * sup_page) {
    printf("Inserting page with address... %p\n", (void *) sup_page->upage);
    printf("Page has offset of... %d\n", sup_page->segment_info->ofs);
    printf("Page has readbytes of... %d\n", sup_page->segment_info->read_bytes);
    printf("Page has zerobytes of ... %d\n\n", sup_page->segment_info->zero_bytes);

    return hash_insert(hash_table, &sup_page->sup_page_table_elem);
}

/*! Copy data to the frame table. */
bool fetch_data_to_frame(struct sup_page *page) {
    bool success = false;
    switch (page->status) {
        case SWAP_PAGE:
            success = get_swap_page(page);
            break;
        case FILE_PAGE:
            success = get_file_page(page);
            break;
        case ZERO_PAGE:
            success = get_zero_page(page);
            break;
    }
    return success;
}

static bool get_swap_page(struct sup_page *page) {
}

/* Will load the page from filesys */
static bool get_file_page(struct sup_page *page) {
    struct segment *segment_info = page->segment_info;
    int read_bytes = segment_info->read_bytes;
    int zero_bytes = segment_info->zero_bytes;
    uint8_t * upage = page->upage;

    struct thread *cur = thread_current();

    file_seek(segment_info->file, segment_info->ofs);
    while (read_bytes > 0 || zero_bytes > 0) {
        /* Calculate how to fill this page.
           We will read PAGE_READ_BYTES bytes from FILE
           and zero the final PAGE_ZERO_BYTES bytes. */
        size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
        size_t page_zero_bytes = PGSIZE - page_read_bytes;
        
        /* Set the frame. */

        struct frame_table_entry * fte = get_frame();
        if (fte == NULL) {
            return false;
        }

        pin(fte);
        uint8_t *kpage = (uint8_t *) fte->frame;

        /* Load the page */
        if (file_read(segment_info->file, kpage, page_read_bytes) != (int) page_read_bytes) {
            /* Read failure */
            unpin(fte);
            free_frame(fte);
            return false;
        }

        memset(kpage + page_read_bytes, 0, page_zero_bytes);
        
        bool set_success = (pagedir_get_page(cur->pagedir, upage) == NULL &&
                            pagedir_set_page(cur->pagedir, upage, fte->frame,
                                            page->segment_info->writable));
        if (!set_success) {
            /* Failure to set pagedir for frame */
            unpin(fte);
            free_frame(fte);
            return false;
        }

        /* Advance */
        read_bytes -= page_read_bytes;
        zero_bytes -= page_zero_bytes;
        upage += PGSIZE;
        unpin(fte);
    }
    printf("Do I make it here even...\n");
    return true;
}

static bool get_zero_page(struct sup_page *page) {
    // memset(fte->frame, 0, PGSIZE);
}
