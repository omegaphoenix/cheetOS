/*! \file page.h
 *
 * Declarations for the supplemental page table
 */

#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <stdbool.h>
#include "filesys/file.h"
#include "threads/thread.h"
#include "vm/frame.h"

#define MAX_STACK 8 * 1024 * 1024 /*!< Maximum stack size in bytes. */
#define NOT_SWAP -1

enum page_status {
    SWAP_PAGE,
    FILE_PAGE,
    ZERO_PAGE
};

/* Container to keep track of file info for loading. */
struct file_info {
    struct file *file;
    off_t offset;
    size_t read_bytes;
    size_t zero_bytes;
};

/*! Pages for supplemental page table. */
struct sup_page {
    void *addr;                           /*!< Address to the virtual page. */
    struct frame_table_entry *fte;        /*!< Frame table entry. */
    enum page_status status;              /*!< Current status of page. */
    int swap_position;                    /*!< Swap position if in swap. -1 otherwise */
    uintptr_t page_no;                    /*!< Page number. */
    struct hash_elem sup_page_table_elem; /*!< Elem for supplemental page table. */
    bool writable;                        /*!< Whether page is writable. */
    struct file_info *file_stats;         /*!< Keep track of file info. */
    bool is_mmap;                         /*!< Page is part of mapped memory */
    bool loaded;                          /*!< If file is already loaded... */
    uint32_t *pagedir;                    /*!< Page directory. */
};

/* Initializes supplemental page hash table */
void thread_sup_page_table_init(struct thread *t);
void thread_sup_page_table_delete(struct thread *t);
struct sup_page *sup_page_file_create(struct file *file, off_t ofs,
    uint8_t *upage, size_t read_bytes, size_t zero_bytes, bool writable);
struct sup_page *sup_page_zero_create(uint8_t *upage, bool writable);
void sup_page_table_delete(struct hash *hash_table);
bool fetch_data_to_frame(struct sup_page *page);

struct sup_page *thread_sup_page_get(struct hash *hash_table, void *addr);
unsigned sup_page_hash(const struct hash_elem *e, void *aux);
bool sup_page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux);
bool sup_page_delete(struct hash *hash_table, void *addr);
void sup_page_delete_page(struct sup_page *page);
void sup_page_insert(struct hash *hash_table, struct sup_page *page);

bool is_stack_access(void *addr, void *esp);
#endif /* vm/page.h */
