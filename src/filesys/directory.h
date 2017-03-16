#ifndef FILESYS_DIRECTORY_H
#define FILESYS_DIRECTORY_H

#include <stdbool.h>
#include <stddef.h>
#include "devices/block.h"

/*! Maximum length of a file name component.
    This is the traditional UNIX maximum length.
    After directories are implemented, this maximum length may be
    retained, but much longer full path names must be allowed. */
#define NAME_MAX 14
#define NUM_ENTRIES 10 /* Temporarily fix number of entries in new directories */

struct inode;

/* Opening and closing directories. */
bool dir_create(block_sector_t sector, size_t entry_cnt);
struct dir *dir_open(struct inode *);
struct dir *dir_open_root(void);
struct dir *dir_reopen(struct dir *);
void dir_close(struct dir *);
struct inode *dir_get_inode(struct dir *);

/* Reading and writing. */
bool dir_lookup(const struct dir *, const char *name, struct inode **);
bool dir_add(struct dir *, const char *name, block_sector_t);
bool dir_remove(struct dir *, const char *name);
bool dir_readdir(struct dir *, char name[NAME_MAX + 1]);

#ifdef CACHE
/* Subdirectories. */
bool is_empty_dir(struct inode *);
bool is_pinned_dir(struct inode *);
void pin_cwd(struct inode *); /* Should only be used in chdir. */
void unpin_cwd(struct inode *); /* Should only be used in chdir. */
#endif

#endif /* filesys/directory.h */

