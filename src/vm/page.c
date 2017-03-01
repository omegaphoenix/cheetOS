#include "vm/page.h"
#include <debug.h>
#include <hash.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "vm/frame.h"

static struct lock read_lock;
static bool install_page(void *upage, void *kpage, bool writable);
static void get_swap_page(struct sup_page *page,
        struct frame_table_entry *fte);
static bool get_file_page(struct sup_page *page,
        struct frame_table_entry *fte);
static void get_zero_page(struct sup_page *page,
        struct frame_table_entry *fte);

void init_sup_page_lock(void) {
    lock_init(&read_lock);
}

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

/*! Create a suplemental page. */
struct sup_page *sup_page_file_create(struct file *file, off_t ofs,
    uint8_t *upage, size_t read_bytes, size_t zero_bytes, bool writable) {
    /* Copy over page data. */
    struct sup_page *page = palloc_get_page(PAL_ZERO);
    if (page == NULL) {
        sys_exit(-1);
    }
    page->addr = upage;
    page->status = FILE_PAGE;
    page->page_no = pg_no(upage);
    page->writable = writable;
    page->kpage = NULL;

    /* Copy over file data. */
    page->file_stats = palloc_get_page(PAL_ZERO);
    page->file_stats->file = file;
    page->file_stats->offset = ofs;
    page->file_stats->read_bytes = read_bytes;
    page->file_stats->zero_bytes = zero_bytes;

    /* Insert into table. */
    struct thread *cur = thread_current();
    sup_page_insert(&cur->sup_page, page);
    return page;
}

/*! Since we're hashing by address, we will be using hash_bytes
    supplied by Pintos. */
unsigned sup_page_hash(const struct hash_elem *e, void *aux UNUSED) {
    const struct sup_page *page = hash_entry(e, struct sup_page, sup_page_table_elem);
    int hash_index = hash_bytes(&page->page_no, sizeof page->page_no);

    return hash_index;
}

/* Hash table's comparison will be using pointer comparisons */
bool sup_page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
    const struct sup_page *first_page = hash_entry(a, struct sup_page, sup_page_table_elem);
    const struct sup_page *second_page = hash_entry(b, struct sup_page, sup_page_table_elem);

    return first_page->page_no < second_page->page_no;
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
    struct sup_page temp_page;
    struct sup_page *return_page = NULL;
    struct hash_elem *temp_elem = NULL;
    temp_page.page_no = pg_no(addr);

    /* Recall that temp_elem is a hash_elem, so we need to hash_entry it */
    temp_elem = hash_find(hash_table, &temp_page.sup_page_table_elem);

    if (temp_elem != NULL) {
        return_page = hash_entry(temp_elem, struct sup_page, sup_page_table_elem);
    }
    return return_page;
}

void sup_page_insert(struct hash *hash_table, struct sup_page *page) {
    hash_insert(hash_table, &page->sup_page_table_elem);
}

/*! Copy data to the frame table. */
void fetch_data_to_frame(struct sup_page *page,
        struct frame_table_entry *fte) {
    bool success;
    switch (page->status) {
        case SWAP_PAGE:
            get_swap_page(page, fte);
            break;
        case FILE_PAGE:
            // lock_acquire(&read_lock);
            success = get_file_page(page, fte);
            // lock_release(&read_lock);
            break;
        case ZERO_PAGE:
            get_zero_page(page, fte);
            break;
    }
}

static void get_swap_page(struct sup_page *page,
        struct frame_table_entry *fte) {
}

/*! Adds a mapping from user virtual address UPAGE to kernel
    virtual address KPAGE to the page table.
    If WRITABLE is true, the user process may modify the page;
    otherwise, it is read-only.
    UPAGE must not already be mapped.
    KPAGE should probably be a page obtained from the user pool
    with palloc_get_page().
    Returns true on success, false if UPAGE is already mapped or
    if memory allocation fails. */
static bool install_page(void *upage, void *kpage, bool writable) {
    struct thread *t = thread_current();

    /* Verify that there's not already a page at that virtual
       address, then map our page there. */
    return (pagedir_get_page(t->pagedir, upage) == NULL &&
            pagedir_set_page(t->pagedir, upage, kpage, writable));
}

static bool get_file_page(struct sup_page *page,
        struct frame_table_entry *fte) {
    uint8_t *kpage = (uint8_t *) fte->frame;
    struct file *file = page->file_stats->file;
    off_t ofs = page->file_stats->offset;
    size_t page_read_bytes = page->file_stats->read_bytes;
    size_t page_zero_bytes = page->file_stats->zero_bytes;
    bool writable = page->writable;
    uint8_t *upage = (uint8_t *) page->addr;
    file_seek(file, ofs);
    if (kpage == NULL) {
        sys_exit(-1);
    }
    ASSERT (page_read_bytes > 0);
    ASSERT (page_read_bytes <= PGSIZE);
    ASSERT (page_read_bytes + page_zero_bytes == PGSIZE);

    int bytes_read = file_read(file, kpage, page_read_bytes);
    if (bytes_read != (int) page_read_bytes) {
        free_frame(kpage);
        sys_exit(-1);
    }
    memset(kpage + page_read_bytes, 0, page_zero_bytes);

    /* Add the page to the process's address space. */
    if (!install_page(upage, kpage, writable)) {
        free_frame(kpage);
        sys_exit(-1);
    }

    return true;
}

static void get_zero_page(struct sup_page *page,
        struct frame_table_entry *fte) {
    memset(fte->frame, 0, PGSIZE);
}
