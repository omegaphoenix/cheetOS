#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/inode.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#ifdef VM
#include "vm/page.h"
#endif

/* Protect filesys calls. */
#ifndef CACHE
static struct lock filesys_lock;
#endif

static void syscall_handler(struct intr_frame *);

/* Helper functions */
void *get_arg(struct intr_frame *f, int num);
void *get_first_arg(struct intr_frame *f);
void *get_second_arg(struct intr_frame *f);
void *get_third_arg(struct intr_frame *f);

/* SYSTEM CALLS */
void sys_halt(void);
pid_t sys_exec(const char *cmd_line);
int sys_wait(pid_t pid);
/* File manipulation */
bool sys_create(const char *file, unsigned initital_size);
bool sys_remove(const char *file);
int sys_open(const char *file);
int sys_filesize(int fd);
int sys_read(int fd, void *buffer, unsigned size);
int sys_write(int fd, void *buffer, unsigned size);
void sys_seek(int fd, unsigned position);
unsigned sys_tell(int fd);
void sys_close(int fd);

#ifdef VM
/* Memory mapping */
mapid_t sys_mmap(int fd, void *addr);
void sys_munmap(mapid_t mapping);
#endif

#ifdef CACHE
/* File system */
bool sys_chdir (const char *dir);
bool sys_mkdir (const char *dir);
bool sys_readdir (int fd, char *name);
bool sys_isdir (int fd);
int sys_inumber (int fd);
#endif

/* User memory access */
static int get_user(const uint8_t *uaddr);
static bool put_user (uint8_t *udst, uint8_t byteput);
static bool valid_read_addr(const void *addr) UNUSED;
static bool valid_write_addr(void *addr) UNUSED;

void syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
#ifndef CACHE
    lock_init(&filesys_lock);
#endif
}

static void syscall_handler(struct intr_frame *f UNUSED) {
    int *fd, *status, *child_pid;
    void **buffer;
    unsigned int *size, *initial_size, *position;
    char **cmd_line;
    char **file;
    char **dir, **name;

    if (f == NULL || !valid_read_addr(f->esp)) {
        sys_exit(ERR);
    }
    /* Get the system call number */
    int syscall_no = *((int *) f->esp);

#ifdef VM
    void **addr;
    mapid_t *mapping;
    thread_current()->esp = f->esp;
#endif

    /* Make the appropriate system call */
    switch (syscall_no) {
        case SYS_HALT:
            sys_halt();
            break;
        case SYS_EXIT:
            status = (int *) get_first_arg(f);
            sys_exit(*status);
            f->eax = *status;
            break;
        case SYS_EXEC:
            cmd_line = (char **) get_first_arg(f);
            f->eax = sys_exec(*cmd_line);
            break;
        case SYS_WAIT:
            child_pid = (pid_t *) get_first_arg(f);
            f->eax = sys_wait(*child_pid);
            break;
        case SYS_CREATE:
            file = (char **) get_first_arg(f);
            initial_size = (unsigned *) get_second_arg(f);
            f->eax = sys_create(*file, *initial_size);
            break;
        case SYS_REMOVE:
            file = (char **) get_first_arg(f);
            f->eax = sys_remove(*file);
            break;
        case SYS_OPEN:
            file = (char **) get_first_arg(f);
            f->eax = sys_open(*file);
            break;
        case SYS_FILESIZE:
            fd = (int *) get_first_arg(f);
            f->eax = sys_filesize(*fd);
            break;
        case SYS_READ:
            fd = (int *) get_first_arg(f);
            buffer = get_second_arg(f);
            size = (unsigned int *) get_third_arg(f);
            f->eax = sys_read(*fd, *buffer, *size);
            break;
        case SYS_WRITE:
            fd = (int *) get_first_arg(f);
            buffer = (void **) get_second_arg(f);
            size = (unsigned int *) get_third_arg(f);
            f->eax = sys_write(*fd, *buffer, *size);
            break;
        case SYS_SEEK:
            fd = (int *) get_first_arg(f);
            position = (unsigned int *) get_second_arg(f);
            sys_seek(*fd, *position);
            break;
        case SYS_TELL:
            fd = (int *) get_first_arg(f);
            f->eax = sys_tell(*fd);
            break;
        case SYS_CLOSE:
            fd = (int *) get_first_arg(f);
            sys_close(*fd);
            break;
#ifdef VM
        case SYS_MMAP:
            fd = (int *) get_first_arg(f);
            addr = (void **) get_second_arg(f);
            f->eax = sys_mmap(*fd, *addr);
            break;
        case SYS_MUNMAP:
            mapping = (mapid_t *) get_first_arg(f);
            sys_munmap(*mapping);
            break;
#endif
#ifdef CACHE
        case SYS_CHDIR:
            dir = get_first_arg(f);
            f->eax = sys_chdir(*dir);
            break;
        case SYS_MKDIR:
            dir = get_first_arg(f);
            f->eax = sys_mkdir(*dir);
            break;
        case SYS_READDIR:
            fd = (int *) get_first_arg(f);
            name = get_second_arg(f);
            f->eax = sys_readdir(*fd, *name);
            break;
        case SYS_ISDIR:
            fd = (int *) get_first_arg(f);
            f->eax = sys_isdir(*fd);
            break;
        case SYS_INUMBER:
            fd = (int *) get_first_arg(f);
            f->eax = sys_inumber(*fd);
            break;
#endif
        default:
            printf("Unimplemented system call number\n");
            sys_exit(ERR);
            break;
    }
}

/*! Returns *num*th argument. Verifies before returning. */
void *get_arg(struct intr_frame *f, int num) {
    void *arg = f->esp + ARG_SIZE * num;
    if (!valid_read_addr(arg)) {
        sys_exit(ERR);
    }
    return arg;
}

void *get_first_arg(struct intr_frame *f) {
    return get_arg(f, 1);
}

void *get_second_arg(struct intr_frame *f) {
    return get_arg(f, 2);
}

void *get_third_arg(struct intr_frame *f) {
    return get_arg(f, 3);
}

/*! Acquire file locks. Need two because bochs might have files and their
    static variables go out of memory. */
void acquire_file_lock(void) {
#ifndef CACHE
    lock_acquire(&filesys_lock);
#endif
}

/*! Release file locks. See comment in acquire_file_lock. */
void release_file_lock(void) {
#ifndef CACHE
    lock_release(&filesys_lock);
#endif
}

/*! Terminates Pintos. Should be seldom used due to loss of information on
    possible deadlock situations, etc. */
void sys_halt(void) {
    shutdown_power_off();
}

/*! Terminates current user program. */
void sys_exit(int status) {
    struct thread *cur = thread_current();
    printf("%s: exit(%d)\n", cur->name, status);
    cur->exit_status = status;

#ifdef VM
    /* Free all mappings. */
    struct list_elem *f;
    while (!list_empty(&cur->mappings)) {
        f = list_begin(&cur->mappings);
        struct mmap_file *mmap =
            list_entry(f, struct mmap_file, mmap_elem);
        sys_munmap(mmap->mapping);
    }
#endif

    /* Free all file buffers. */
    struct list_elem *e;
    while (!list_empty(&cur->open_files)) {
        e = list_begin(&cur->open_files);
        struct sys_file *open_file =
            list_entry(e, struct sys_file, file_elem);
        sys_close(open_file->fd);
    }
    thread_exit();
}

/*! Run executable and return new pid. Return ERR if program cannot
    load or run for any reason. */
pid_t sys_exec(const char *cmd_line) {
    if (!valid_read_addr(cmd_line)) {
        return ERR;
    }
    struct thread *cur = thread_current();

    /* File system call */
    acquire_file_lock();
    pid_t new_process_pid = process_execute(cmd_line);

    /* Wait for executable to load. */
    release_file_lock();

    if (!cur->loaded) {
        /* Executable failed to load. */
        return ERR;
    }
    return new_process_pid;
}

/*! Wait for a child process pid and retrive the child's exit status. */
int sys_wait(pid_t pid) {
    int exit_status = process_wait(pid);
    return exit_status;
}

/*! Create new file called *file* initially *initial_size* bytes in size.
    Returns true if successful. */
bool sys_create(const char *file, unsigned initial_size) {
    if (!valid_read_addr((void *) file)) {
        sys_exit(ERR);
    }

    /* File system call */
    acquire_file_lock();
    bool success = filesys_create(file, initial_size);
    release_file_lock();

    return success;
}

/*! Delete file called *file*. Return true if successful. */
bool sys_remove(const char *file) {
    if (!valid_read_addr((void *) file)) {
        sys_exit(ERR);
    }

    /* File system call */
    acquire_file_lock();
    bool success = filesys_remove(file);
    release_file_lock();

    return success;
}

/*! Open the file called *file*. Returns ERR if file could not be opened. */
int sys_open(const char *file) {
    if (!valid_read_addr((void *) file)) {
        sys_exit(ERR);
    }
    struct thread *cur = thread_current();

    /* File system call */
    acquire_file_lock();
    struct file *open_file = filesys_open(file);
    if (open_file == NULL) {
        release_file_lock();
        return ERR;
    }
    int fd = next_fd(cur);
    fd = add_open_file(cur, open_file, fd);
    release_file_lock();

    ASSERT(fd >= CONSOLE_FD || fd == ERR);
    return fd;
}

/*! Returns the size, in bytes, of the file open as fd. */
int sys_filesize(int fd) {
    struct thread *cur = thread_current();
    struct file *open_file = get_fd(cur, fd);

    /* File system call */
    acquire_file_lock();
    int size = file_length(open_file);
    release_file_lock();

    return size;
}

/*! Read *size* bytes from file open as fd into buffer. Return the number of
    bytes actually read, 0 at end of file, or -1 if file could not be read. */
int sys_read(int fd, void *buffer, unsigned size) {
    if (!valid_write_addr(buffer) || !valid_write_addr(buffer + size)) {
        sys_exit(ERR);
    }
    size_t bytes_read = 0;
    /* Pointer to current position in buffer */
    char *buff = (char *) buffer;
    struct thread *cur = thread_current();

    if (fd == STDIN_FILENO) {
        /* Read from keyboard input */
        while ((unsigned) bytes_read < size) {
            *buff = input_getc();
            buff++;
            bytes_read++;
        }
    } else if (is_existing_fd(cur, fd)) {
#ifdef VM
        size_t bytes_left = size;
        void *temp_buff = buff;
        while (bytes_left > 0) {
            size_t offset = temp_buff - pg_round_down(temp_buff);
            bool success = false;
            struct sup_page *page = thread_sup_page_get(&cur->sup_page,
                    temp_buff - offset);

            if (page == NULL) {
                /* Handle stack access. */
                if (is_stack_access(temp_buff, cur->esp)) {
                    page = sup_page_zero_create(temp_buff - offset, true);
                    success = fetch_data_to_frame(page);
                    page->status = SWAP_PAGE;
                }
            }
            else {
                if (page->loaded) {
                    pin(page->fte);
                }
                else {
                    success = fetch_data_to_frame(page);
                }
            }
            ASSERT(success || (page != NULL && page->loaded));

            /* Calculate how many bytes we can write for this page. */
            size_t read_bytes = offset + bytes_left;
            if (read_bytes > PGSIZE) {
                read_bytes = PGSIZE - offset;
            }
            else {
                read_bytes = bytes_left;
            }

            struct file *open_file = get_fd(cur, fd);
            if (open_file == NULL) {
                return ERR;
            }

            ASSERT(page->loaded);
            /* File system call */
            acquire_file_lock();
            bytes_read += file_read(open_file, temp_buff, read_bytes);
            release_file_lock();

            /* Update remaining bytes. */
            bytes_left -= read_bytes;
            temp_buff += read_bytes;

            /* Done with frame. */
            unpin(page->fte);
        }
#else
        /* File system call */
        acquire_file_lock();
        struct file *open_file = get_fd(cur, fd);
        if (open_file == NULL) {
            release_file_lock();
            return ERR;
        }

        bytes_read = file_read(open_file, buffer, size);
        release_file_lock();
#endif
    } else {
        sys_exit(ERR);
    }

    return bytes_read;
}

/*! Writes size bytes from buffer to the open file fd. Returns the number of
    bytes actually written.
    Writing past end-of-file would normally extend the file, but file growth
    is not implemented by the basic file system. The expected behavior is to
    write as many bytes as possible up to end-of-file and return the actual
    number written, or 0 if no bytes could be written at all.
    Fd 1 writes to the console. */
int sys_write(int fd, void *buffer, unsigned size) {
    if (!valid_read_addr(buffer) || !valid_read_addr(buffer + size)) {
        sys_exit(ERR);
    }
    int bytes_written = 0;

    struct thread *cur = thread_current();

    if (fd == STDOUT_FILENO) {
        /* Write to console */
        size_t block_size = MAX_BUF_WRI;

        /* If size greater than several hundred bytes, break up */
        while (size > bytes_written + block_size) {
            putbuf((char *)(buffer + bytes_written), block_size);
            bytes_written += block_size;
        }

        /* Write remaining bytes */
        putbuf(buffer + bytes_written, size - bytes_written);
        bytes_written = size;
    } else if (is_existing_fd(cur, fd)) {
#ifdef VM
        size_t bytes_left = size;
        void *temp_buff = buffer;
        while (bytes_left > 0) {
            size_t offset = temp_buff - pg_round_down(temp_buff);
            bool success = false;
            struct sup_page *page = thread_sup_page_get(&cur->sup_page,
                    temp_buff - offset);

            if (page == NULL) {
                /* Handle stack access. */
                if (is_stack_access(temp_buff, cur->esp)) {
                    page = sup_page_zero_create(temp_buff - offset, true);
                    success = fetch_data_to_frame(page);
                    page->status = SWAP_PAGE;
                }
            }
            else {
                if (page->loaded) {
                    pin(page->fte);
                }
                else {
                    success = fetch_data_to_frame(page);
                    ASSERT(success);
                }
            }
            ASSERT(success || (page != NULL && page->loaded));

            struct file *open_file = get_fd(cur, fd);
            if (open_file == NULL) {
                sys_exit(ERR);
            }

            /* Calculate how many bytes we can write for this page. */
            size_t write_bytes = offset + bytes_left;
            if (write_bytes > PGSIZE) {
                write_bytes = PGSIZE - offset;
            }
            else {
                write_bytes = bytes_left;
            }

            ASSERT(page->loaded);
            /* File system call */
            acquire_file_lock();
            bytes_written += file_write(open_file, temp_buff, write_bytes);
            release_file_lock();

            /* Update remaining bytes. */
            bytes_left -= write_bytes;
            temp_buff += write_bytes;

            /* Done with frame. */
            unpin(page->fte);
        }
#else
        acquire_file_lock();
        struct file *open_file = get_fd(cur, fd);
        if (open_file == NULL) {
            release_file_lock();
            sys_exit(ERR);
        }

        /* File system call */
        bytes_written = file_write(open_file, buffer, size);
        release_file_lock();
#endif
    } else {
        sys_exit(ERR);
    }

    return bytes_written;
}

/*! Changes next byte to be read or written in open file *fd* to *position*,
    expressed in bytes from beginning of file. */
void sys_seek(int fd, unsigned position) {
    struct thread *cur = thread_current();

    struct file *open_file = get_fd(cur, fd);
    if (open_file == NULL) {
        sys_exit(ERR);
    }

    /* File system call */
    acquire_file_lock();
    file_seek(open_file, position);
    release_file_lock();
}


/*! Return position of next byte to be read or written in open file fd,
    expressed in bytes from beginning of file. */
unsigned sys_tell(int fd) {
    struct thread *cur = thread_current();

    struct file *open_file = get_fd(cur, fd);
    if (open_file == NULL) {
        sys_exit(ERR);
    }

    /* File system call */
    acquire_file_lock();
    unsigned position = file_tell(open_file);
    release_file_lock();

    return position;
}

/*! Close file descriptor fd. */
void sys_close(int fd) {
    struct thread *cur = thread_current();

    struct file *open_file = get_fd(cur, fd);
    if (open_file == NULL) {
        sys_exit(ERR);
    }

    /* File system call */
    acquire_file_lock();
    /* Delete file from thread */
    close_fd(cur, fd);
    release_file_lock();
}

#ifdef VM
/*! Maps the file open as FD into the process's virtual address space.
    The entire file is mapped as consecutive pages starting at ADDR.
    Returns mapping id that is unique within the process, or -1 on failure. */
mapid_t sys_mmap (int fd, void *addr) {
    struct thread *cur = thread_current();
    struct file *open_file = file_reopen(get_fd(cur, fd));

    /* Cannot map FD 0 or FD 1 */
    if (fd == 0 || fd == 1) {
        return ERR;
    }
    /* File must be not NULL and file must have length > 0 */
    if (open_file == NULL || file_length(open_file) == 0) {
        return ERR;
    }
    /* ADDR must not be 0 and ADDR must be page aligned */
    if (addr == 0 || pg_ofs(addr) != 0) {
        return ERR;
    }

    /* Page range cannot overlap with other pages in use */
    void *upage = addr;
    while (upage < addr + file_length(open_file)) {
        if (thread_sup_page_get(&cur->sup_page, upage) != NULL) {
            return ERR;
        }
        upage += PGSIZE;
    }

    /* Load file to virtual address */
    upage = addr;
    uint32_t read_bytes = file_length(open_file);
    off_t offset = 0;
    bool writable = true;
    while (read_bytes > 0) {
        size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
        size_t page_zero_bytes = PGSIZE - page_read_bytes;

        ASSERT(thread_sup_page_get(&cur->sup_page, upage) == NULL);
        struct sup_page *page = sup_page_file_create(open_file, offset, upage,
                page_read_bytes, page_zero_bytes, writable);

        if (page == NULL) {
            return ERR;
        }

        /* Flag indicates that when evicted, write back to file */
        page->is_mmap = true;
        page->status = FILE_PAGE;

        read_bytes -= page_read_bytes;
        upage += PGSIZE;
        offset += PGSIZE;
    }

    /* Add mapping to thread's mappings list and return unique mapping id */
    int mapping = next_mapping(cur);
    return add_mmap(cur, addr, fd, mapping);
}

/*! Unmaps mmapped file by writing file back to disk and evicting frame. */
void sys_munmap (mapid_t mapping) {
    struct thread *cur = thread_current();

    struct mmap_file *mmap = get_mmap(cur, mapping);
    if (mmap == NULL) {
        printf("no mmap file found!\n");
        sys_exit(ERR);
    }
    void *upage = mmap->addr;
    struct sup_page *page;

    off_t zero_bytes = 0;

    while (zero_bytes == 0) {
        /* Write dirty pages back to the file */
        page = thread_sup_page_get(&cur->sup_page, upage);

        if (page == NULL) {
            break;
        }
        zero_bytes = page->file_stats->zero_bytes;

        /* Delete page */
        if (page->loaded) {
            ASSERT(page->fte != NULL);
            ASSERT(page->is_mmap);
            ASSERT(page->fte->pin_count == 0);
            evict_chosen_frame(page->fte, false);
        }
        sup_page_delete(&cur->sup_page, upage);
        ASSERT(thread_sup_page_get(&cur->sup_page, upage) == NULL);

        upage += PGSIZE;
    }

    /* Remove entry from list of mmap files */
    remove_mmap(cur, mapping);

}
#endif

#ifdef CACHE
/*! Changes the current working directory of the process to DIR, which may be
    relative or absolute. Returns true if successful, false on failure. */
bool sys_chdir (const char *dir) {
    bool success = false;
    char *name = NULL;
    struct dir *parent_dir = NULL;
    struct inode *inode = NULL;
    struct thread *cur = thread_current();

    parse_path(dir, parent_dir, name);
    success = dir_lookup(dir, name, &inode);
    if (!success || !is_dir(inode)) {
        return false;
    }
    // might need to close old directory?
    cur->cur_dir_inode = inode;
    return true;
}

/*! Creates the directory named DIR, which may be relative or absolute.
    Returns true if successful, false on failure. Fails if DIR already exists
    or if any directory name in DIR, besides the last, doesn't already exist. */
bool sys_mkdir (const char *dir) {
    // TODO
    return false;
}

/*! Reads a directory entry from file descriptor FD, which must represent a
    directory. If successful, stores the null-terminated file name in NAME,
    which must have room for READDIR_MAX_LEN + 1 bytes, and returns true.
    If no entries are left in the directory, returns false. */
bool sys_readdir (int fd, char *name) {
    // TODO
    return false;
}

/*! Returns true if fd represents a directory, false if it represents an
    ordinary file.*/
bool sys_isdir (int fd) {
    // TODO
    return false;
}

/*! Returns the inode number of the inode associated with fd, which may
    represent an ordinary file or a directory. */
int sys_inumber (int fd) {
    // TODO
    return 0;
}
#endif

/* Returns true if addr is valid for reading */
static bool valid_read_addr(const void *addr) {
    /* Check that address is below PHYS_BASE
       and then attempt to read a byte at the address */
    return addr != NULL && is_user_vaddr(addr) && (get_user(addr) != ERR);
}

/* Returns true if addr is valid for writing */
static bool valid_write_addr(void *addr) {
    /* Check that address is below PHYS_BASE
       and then attempt to write a byte '1' to the address */
    return addr != NULL && is_user_vaddr(addr) && put_user(addr, 1);
}

/*! Reads a byte at user virtual address UADDR.
    UADDR must be below PHYS_BASE.
    Returns the byte value if successful, -1 if a segfault occurred. */
static int get_user(const uint8_t *uaddr) {
    int result;
    asm ("movl $1f, %0; movzbl %1, %0; 1:"
         : "=&a" (result) : "m" (*uaddr));
    return result;
}

/*! Writes BYTE to user address UDST.
    UDST must be below PHYS_BASE.
    Returns true if successful, false if a segfault occurred.
    If written, we will write back to previous byte. */
static bool put_user (uint8_t *udst, uint8_t byte) {
    int prev_byte = get_user(udst);

    if (prev_byte == -1) {
        return false;
    }

    int error_code;
    asm ("movl $1f, %0; movb %b2, %1; 1:"
         : "=&a" (error_code), "=m" (*udst) : "q" (byte));

    bool can_write = (error_code != ERR);
    if (can_write) {
        /* Assuming this can be written already */
        asm ("movl $1f, %0; movb %b2, %1; 1:"
         : "=&a" (error_code), "=m" (*udst) : "q" (prev_byte));
    }

    return can_write;
}
