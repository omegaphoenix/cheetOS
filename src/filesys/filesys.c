#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/cache.h"
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "threads/malloc.h"
#include "threads/thread.h"

/*! Partition that contains the file system. */
struct block *fs_device;

static void do_format(void);

/*! Initializes the file system module.
    If FORMAT is true, reformats the file system. */
void filesys_init(bool format) {
    cache_table_init();
    fs_device = block_get_role(BLOCK_FILESYS);
    if (fs_device == NULL)
        PANIC("No file system device found, can't initialize file system.");

    inode_init();
    directory_init();
    free_map_init();

    if (format)
        do_format();

    free_map_open();

    /* Init current directory for current thread */
    struct inode *inode = inode_open(ROOT_DIR_SECTOR);
    thread_current()->cur_dir_inode = inode;
    inc_in_use(inode);
    init_subdir(inode, NULL);

}

/*! Shuts down the file system module, writing any unwritten data to disk. */
void filesys_done(void) {
    write_all_dirty();
    free_map_close();
}

/*! Creates a file at path PATH with the given INITIAL_SIZE.  Returns true if
    successful, false otherwise.  Fails if a file at PATH already exists,
    or if internal memory allocation fails. */
bool filesys_create(const char *path, off_t initial_size) {
    block_sector_t inode_sector = 0;
    struct dir *dir = NULL;
    char *name = NULL;

    /* Copy path to be safe */
    char *path_copy = calloc(MAX_PATH_SIZE, 1);
    if (path_copy == NULL) {
        return false;
    }
    strlcpy(path_copy, path, strlen(path) + 1);
    parse_path(path_copy, &dir, &name);

    bool success = (dir != NULL &&
                    free_map_allocate(1, &inode_sector) &&
                    inode_create(inode_sector, initial_size) &&
                    dir_add(dir, name, inode_sector));

    if (!success && inode_sector != 0)
        free_map_release(inode_sector, 1);
    dir_close(dir);
    free(path_copy);
    return success;
}

/*! Opens the file with the given PATH.  Returns the new file if successful
    or a null pointer otherwise.  Fails if no file with path PATH exists,
    or if an internal memory allocation fails. */
struct file * filesys_open(const char *path) {
    struct dir *dir = NULL;
    struct inode *inode = NULL;
    char *name = NULL;

    /* Special case: '/' is root, no need to call dir_lookup */
    if (!strcmp(path, "/")) {
        return file_open(inode_open(ROOT_DIR_SECTOR));
    }

    /* Copy path to be safe */
    char *path_copy = calloc(MAX_PATH_SIZE, 1);
    if (path_copy == NULL) {
        return NULL;
    }
    strlcpy(path_copy, path, strlen(path) + 1);
    parse_path(path_copy, &dir, &name);

    if (dir != NULL)
        dir_lookup(dir, name, &inode);
    dir_close(dir);

    free(path_copy);
    return file_open(inode);
}

/*! Deletes the file at PATH.  Returns true if successful, false on failure.
    Fails if no file at PATH exists, or if an internal memory allocation
    fails. */
bool filesys_remove(const char *path) {
    struct dir *dir = NULL;
    char *name = NULL;

    /* Copy path to be safe */
    char *path_copy = calloc(MAX_PATH_SIZE, 1);
    if (path_copy == NULL) {
        return false;
    }
    strlcpy(path_copy, path, strlen(path) + 1);
    parse_path(path_copy, &dir, &name);

    bool success = dir != NULL && dir_remove(dir, name);
    dir_close(dir);

    return success;
}

/*! Parses PATH to find the file name and the directory that contains the file.
    These values are put in NAME and DIR respectively. Values may be NULL.
    Returns true if path is valid, i.e, all directories along path exist. The
    file itself may or may not exist. */
bool parse_path(char *path, struct dir **dir, char **name) {
    struct inode *inode = NULL;
    bool prev_was_null = false;
    char *token, *save_ptr;
    struct inode *prev_dir_inode = NULL;
    *name = ""; /* In case of empty path */

    /* Parse path */
    if (path[0] == '/') {
        /* Begin at root directory */
        *dir = dir_open_root();
    }
    else {
        /* Begin at current directory */
        if (thread_current()->cur_dir_inode == NULL) {
            *dir = dir_open_root();
        }
        else {
            *dir = dir_open(thread_current()->cur_dir_inode);
        }
    }

    for (token = strtok_r(path, "/", &save_ptr); token != NULL;
        token = strtok_r(NULL, "/", &save_ptr)) {
        if (prev_was_null) { /* missing directory along path */
            *name = NULL;
            *dir = NULL;
            return false;
        }
        if (prev_dir_inode != NULL) {
            dir_close(*dir);
            *dir = dir_open(prev_dir_inode);
        }
        ASSERT(*dir != NULL);
        dir_lookup(*dir, token, &inode);

        if (inode == NULL) { /* File or dir is missing */
            prev_was_null = true;
        }

        else if (is_dir(inode)) {
            prev_dir_inode = inode; /* Open directory when parsing next token */
        }

        *name = token;
    }

    return true;

}

/*! Formats the file system. */
static void do_format(void) {
    printf("Formatting file system...");
    free_map_create();
    if (!dir_create(ROOT_DIR_SECTOR, 16))
        PANIC("root directory creation failed");
    free_map_close();
    printf("done.\n");
}
