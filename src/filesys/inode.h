#ifndef FILESYS_INODE_H
#define FILESYS_INODE_H

#include <stdbool.h>
#include "filesys/off_t.h"
#include "devices/block.h"

/* Each inode_disk will have 126 direct, 1 indirect, and 1 double indirect
 * 124 direct indices + 128 indirect indices + 128 ^ 2 double indirect indices
 * = 16636 indices > 16384 */
#define DIRECT_BLOCK_COUNT 124
#define TOTAL_SECTOR_COUNT 128

struct bitmap;

void inode_init(void);
bool inode_create(block_sector_t, off_t);
struct inode *inode_open(block_sector_t);
struct inode *inode_reopen(struct inode *);
block_sector_t inode_get_inumber(const struct inode *);
void inode_close(struct inode *);
void inode_remove(struct inode *);
off_t inode_read_at(struct inode *, void *, off_t size, off_t offset);
off_t inode_write_at(struct inode *, const void *, off_t size, off_t offset);
void inode_deny_write(struct inode *);
void inode_allow_write(struct inode *);
off_t inode_length(const struct inode *);

#ifdef CACHE
bool is_dir(const struct inode *inode);
void set_dir(struct inode *inode, bool is_dir);
#endif 

#endif /* filesys/inode.h */
