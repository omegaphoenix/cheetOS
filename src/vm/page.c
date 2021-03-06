#include "vm/page.h"
#include <debug.h>
#include <hash.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "vm/frame.h"
#include "vm/swap.h"

static bool install_page(void *upage, void *kpage, bool writable);
static bool get_swap_page(struct sup_page *page,
        struct frame_table_entry *fte);
static bool get_file_page(struct sup_page *page,
        struct frame_table_entry *fte);
static bool get_zero_page(struct sup_page *page,
        struct frame_table_entry *fte);
static void sup_page_free(struct hash_elem *e, void *aux);

/*! Initialize supplemental page table. */
void thread_sup_page_table_init(struct thread *t) {
    /* For this simple hash table, no auxiliary data should be necessary */
    hash_init(&t->sup_page, sup_page_hash, sup_page_less, NULL);
}

/*! Frees a sup_page element. */
static void sup_page_free(struct hash_elem *e, void *aux UNUSED) {
    struct sup_page *page_to_delete = hash_entry(e, struct sup_page,
            sup_page_table_elem);

    ASSERT(page_to_delete != NULL);
    struct frame_table_entry *fte = page_to_delete->fte;
    ASSERT(fte == NULL || fte->pin_count == 0);
    if (page_to_delete->loaded && fte != NULL) {
        evict_chosen_frame(fte, true);
    }
    /* First, free the file stats */
    free(page_to_delete->file_stats);
    /* Then free page_to_delete */
    free(page_to_delete);
    page_to_delete = NULL;
}

/*! Frees a hash table */
void thread_sup_page_table_delete(struct thread *t) {
    acquire_eviction_lock();
    hash_destroy(&t->sup_page, sup_page_free);
    release_eviction_lock();
}

/*! Create a suplemental page. */
struct sup_page *sup_page_file_create(struct file *file, off_t ofs,
    uint8_t *upage, size_t read_bytes, size_t zero_bytes, bool writable) {
    struct thread *cur = thread_current();
    ASSERT (read_bytes <= PGSIZE);
    ASSERT (read_bytes + zero_bytes == PGSIZE);

    /* Copy over page data. */
    struct sup_page *page = malloc(sizeof(struct sup_page));
    if (page == NULL) {
        printf("Not enough space!\n");
        sys_exit(-1);
    }
    struct file_info *file_stats = malloc(sizeof(struct file_info));
    if (file_stats == NULL) {
        printf("Not enough space!\n");

        free(page);
        sys_exit(-1);
    }

    page->addr = upage;
    ASSERT(pg_ofs(upage) == 0);
    if (read_bytes > 0) {
        page->status = FILE_PAGE;
    }
    else {
        page->status = ZERO_PAGE;
    }
    page->page_no = pg_no(page->addr);
    page->writable = writable;
    page->fte = NULL;
    page->swap_position = NOT_SWAP; /* Not in swap yet */
    page->loaded = false;

    /* Copy over file data. */
    page->file_stats = file_stats;
    page->file_stats->file = file;
    page->file_stats->offset = ofs;
    page->file_stats->read_bytes = read_bytes;
    page->file_stats->zero_bytes = zero_bytes;
    page->pagedir = cur->pagedir;

    /* Default is_mmap to false; set this flag in sys_mmap(). */
    page->is_mmap = false;

    /* Insert into table. */
    sup_page_insert(&cur->sup_page, page);
    return page;
}

/*! Create a suplemental page of zeros. */
struct sup_page *sup_page_zero_create(uint8_t *upage, bool writable) {
    struct thread *cur = thread_current();
    /* Copy over page data. */
    struct sup_page *page = malloc(sizeof(struct sup_page));
    if (page == NULL) {
        sys_exit(-1);
    }
    struct file_info *file_stats = malloc(sizeof(struct file_info));
    if (file_stats == NULL) {
        printf("Not enough space!\n");

        free(page);
        sys_exit(-1);
    }
    page->addr = upage;
    ASSERT(pg_ofs(upage) == 0);
    page->status = ZERO_PAGE;
    page->page_no = pg_no(page->addr);
    page->writable = writable;
    page->fte = NULL;
    page->swap_position = NOT_SWAP; /* Not in swap yet */
    page->loaded = false;

    /* Copy over file data. */
    page->file_stats = file_stats;
    page->file_stats->file = NULL;
    page->file_stats->offset = 0;
    page->file_stats->read_bytes = 0;
    page->file_stats->zero_bytes = PGSIZE;
    page->pagedir = cur->pagedir;

    /* Default is_mmap to false; set this flag in sys_mmap(). */
    page->is_mmap = false;

    /* Insert into table. */
    sup_page_insert(&cur->sup_page, page);
    return page;
}

/*! Since we're hashing by address, we will be using hash_bytes
    supplied by Pintos. */
unsigned sup_page_hash(const struct hash_elem *e, void *aux UNUSED) {
    const struct sup_page *page = hash_entry(e, struct sup_page, sup_page_table_elem);
    int hash_index = hash_int(page->page_no);

    return hash_index;
}

/*! Hash table's comparison will be using pointer comparisons */
bool sup_page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
    const struct sup_page *first_page = hash_entry(a, struct sup_page, sup_page_table_elem);
    const struct sup_page *second_page = hash_entry(b, struct sup_page, sup_page_table_elem);

    return first_page->page_no < second_page->page_no;
}

/*! Delete an entry from hash table using the address of the page */
bool sup_page_delete(struct hash *hash_table, void *addr) {
    acquire_eviction_lock();
    struct sup_page temp_page;
    struct hash_elem *elem_to_delete = NULL;
    struct hash_elem *deleted_elem = NULL;

    temp_page.page_no = pg_no(addr);

    elem_to_delete = hash_find(hash_table, &temp_page.sup_page_table_elem);

    /* Found the element inside the hash table.*/
    if (elem_to_delete != NULL) {
        /* Deleted elem must exist because elem_to_delete is inside hash table */
        deleted_elem = hash_delete(hash_table, elem_to_delete);

        /* Defensively check to see if it's still inside the hash_table/deleted_elem is not null */
        ASSERT(deleted_elem != NULL);
        ASSERT(thread_sup_page_get(hash_table, addr) == NULL);

        /* Free the element */
        sup_page_free(deleted_elem, NULL);

        release_eviction_lock();
        return true;
    }
    release_eviction_lock();
    return false;
}

/*! Delete an entry from hash table using the address of the page */
void sup_page_delete_page(struct sup_page *page) {
    struct thread *cur = thread_current();
    bool success = sup_page_delete(&cur->sup_page, page->addr);
    if (!success) {
        PANIC("sup_page_free failed\n");
    }
}

/*! Retrieves a supplemental page from the hash table via address */
struct sup_page *thread_sup_page_get(struct hash *hash_table, void *addr) {
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

/*! Insert page into supplemental hash table. */
void sup_page_insert(struct hash *hash_table, struct sup_page *page) {
    hash_insert(hash_table, &page->sup_page_table_elem);
}

/*! Copy data to the frame table. */
bool fetch_data_to_frame(struct sup_page *page) {
    ASSERT(!page->loaded);
    struct frame_table_entry *fte = get_frame();

    bool success = false;
    if (page->loaded) {
        return page->loaded;
    }

    pagedir_clear_page(page->pagedir, page->addr);

    /* Loads a page */
    switch (page->status) {
        case SWAP_PAGE:
            success = get_swap_page(page, fte);
            break;
        case FILE_PAGE:
            success = get_file_page(page, fte);
            break;
        case ZERO_PAGE:
            success = get_zero_page(page, fte);
            break;
    }

    uint32_t *pagedir = thread_current()->pagedir;
    ASSERT (pagedir != NULL);
    page->pagedir = pagedir;
    page->fte = fte;
    fte->spte = page;
    fte->addr = page->addr;
    fte->pagedir = page->pagedir;

    if (success) {
        page->loaded = true;
    }
    pagedir_set_dirty(pagedir, fte->addr, false);
    pagedir_set_accessed(pagedir, fte->addr, true);
    return success;
}

/*! Load the a swap page into memory. */
static bool get_swap_page(struct sup_page *page,
        struct frame_table_entry *fte) {
    return swap_table_in(page, fte);
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

/*! Load the file page into memory. */
static bool get_file_page(struct sup_page *page,
        struct frame_table_entry *fte) {
    /* Get physical address. */
    uint8_t *kpage = (uint8_t *) fte->frame;
    if (kpage == NULL) {
        return false;
    }

    /* Get variables. */
    bool writable = page->writable;
    uint8_t *upage = (uint8_t *) page->addr;

    /* Get file variables set during load_segment. */
    struct file *file = page->file_stats->file;
    off_t ofs = page->file_stats->offset;
    size_t page_read_bytes = page->file_stats->read_bytes;
    size_t page_zero_bytes = page->file_stats->zero_bytes;

    ASSERT (page_read_bytes <= PGSIZE);
    ASSERT (page_read_bytes + page_zero_bytes == PGSIZE);

    /* Read file. */
    acquire_file_lock();
    int bytes_read = file_read_at(file, kpage, page_read_bytes, ofs);
    release_file_lock();
    if (bytes_read != (int) page_read_bytes) {
        return false;
    }

    /* Zero out bytes. */
    memset(kpage + page_read_bytes, 0, page_zero_bytes);

    /* Add the page to the process's address space. */
    if (!install_page(upage, kpage, writable)) {
        return false;
    }

    return true;
}

/*! Load the a page of zeros into memory. */
static bool get_zero_page(struct sup_page *page,
        struct frame_table_entry *fte) {
    /* Get physical address. */
    uint8_t *kpage = (uint8_t *) fte->frame;

    /* Get variables. */
    bool writable = page->writable;
    uint8_t *upage = (uint8_t *) page->addr;

    /* Get file variables set during load_segment. */
    size_t page_read_bytes = page->file_stats->read_bytes;
    size_t page_zero_bytes = page->file_stats->zero_bytes;

    ASSERT (page_read_bytes == 0);
    ASSERT (page_zero_bytes == PGSIZE);

    /* Zero out bytes. */
    memset(kpage, 0, PGSIZE);

    /* Add the page to the process's address space. */
    if (!install_page(upage, kpage, writable)) {
        return false;
    }

    return true;
}

/*! Return true addr appears to be a stack address. */
bool is_stack_access(void *addr, void *esp) {
    /* Buggy if user program writes to stack below stack pointer. */
    if (!((addr >= (void *) (esp - 32)) && (addr < PHYS_BASE))) {
        return false;
    }
    int size = PHYS_BASE - addr;
    if (size > MAX_STACK) {
        sys_exit(-1);
    }
    return true;
}
