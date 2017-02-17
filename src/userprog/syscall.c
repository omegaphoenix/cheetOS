#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/process.h"

struct semaphore *exec_lock, *filesys_lock;

static void syscall_handler(struct intr_frame *);

/* Helper functions */
void *get_arg(struct intr_frame *f, int num);
void *get_first_arg(struct intr_frame *f);
void *get_second_arg(struct intr_frame *f);
void *get_third_arg(struct intr_frame *f);

/* SYSTEM CALLS */
void sys_halt(void);
void sys_exit(int status);
pid_t sys_exec(const char *cmd_line);
int sys_wait(pid_t pid);
/* File manipulation */
bool sys_create(const char *file, unsigned initital_size);
bool sys_remove(const char *file);
bool sys_open(const char *file);
int sys_filesize(int fd);
int sys_read(int fd, void *buffer, unsigned size);
int sys_write(int fd, const void *buffer, unsigned size);
void sys_seek(int fd, unsigned position);
unsigned sys_tell(int fd);
void sys_close(int fd);

/* User memory access */
static int get_user(const uint8_t *uaddr);
static bool put_user (uint8_t *udst, uint8_t byteput);
static bool valid_read_addr(const void *addr) UNUSED;
static bool valid_write_addr(void *addr) UNUSED;

void syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
    struct semaphore execute_lock, filesystem_lock;
    exec_lock = &execute_lock;
    sema_init(exec_lock, 1);

    filesys_lock = &filesystem_lock;
    sema_init(filesys_lock, 1);
}

static void syscall_handler(struct intr_frame *f UNUSED) {
    int *fd, *status, *child_pid;
    void **buffer;
    unsigned int *size, *initial_size, *position;
    char *cmd_line, *file;

    /* Get the system call number */
    if (f == NULL || !valid_read_addr(f->esp)) {
        sys_exit(ERR);
        return;
    }
    int syscall_no = *((int *) f->esp);

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
            cmd_line = (char *) get_first_arg(f);
            f->eax = sys_exec(cmd_line);
            break;
        case SYS_WAIT:
            child_pid = (pid_t *) get_first_arg(f);
            f->eax = sys_wait(*child_pid);
            break;
        case SYS_CREATE:
            file = (char *) get_first_arg(f);
            initial_size = (unsigned *) get_second_arg(f);
            f->eax = sys_create(file, *initial_size);
            break;
        case SYS_REMOVE:
            file = (char *) get_first_arg(f);
            f->eax = sys_remove(file);
            break;
        case SYS_OPEN:
            file = (char *) get_first_arg(f);
            f->eax = sys_open(file);
            break;
        case SYS_FILESIZE:
            fd = (int *) get_first_arg(f);
            f->eax = sys_filesize(*fd);
            break;
        case SYS_READ:
            fd = (int *) get_first_arg(f);
            buffer = get_second_arg(f);
            size = (unsigned int *) get_third_arg(f);
            f->eax = sys_read(*fd, buffer, *size);
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
        return NULL;
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

/*! Terminates Pintos. Should be seldom used due to loss of information on
    possible deadlock situations, etc. */
void sys_halt(void) {
    shutdown_power_off();
}

/*! Terminates current user program. */
void sys_exit(int status) {
    printf("%s:exit(%d)\n", thread_current()->name, status);
    thread_exit();
}

/*! Run executable and return new pid. Return ERR if program cannot
    load or run for any reason. */
pid_t sys_exec(const char *cmd_line) {
    if (cmd_line == NULL) {
        return ERR;
    }
    sema_down(exec_lock);
    pid_t new_process_pid = process_execute(cmd_line);
    sema_up(exec_lock);
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
    bool success = filesys_create(file, initial_size);
    return success;
}

/*! Delete file called *file*. Return true if successful. */
bool sys_remove(const char *file) {
    sema_down(filesys_lock);
    bool success = filesys_remove(file);
    sema_up(filesys_lock);
    return success;
}

/*! Open the file called *file*. Returns ERR if file could not be opened. */
bool sys_open(const char *file) {
    sema_down(filesys_lock);
    struct file *open_file = filesys_open(file);
    sema_up(filesys_lock);
    struct thread *cur = thread_current();
    int fd = next_fd(cur);
    fd = add_open_file(cur, open_file, fd);
    return fd;
}

/*! Returns the size, in bytes, of the file open as fd. */
int sys_filesize(int fd) {
    struct thread *cur = thread_current();
    struct file *open_file = get_fd(cur, fd);
    sema_down(filesys_lock);
    int size = file_length(open_file);
    sema_up(filesys_lock);
    return size;
}

/*! Read *size* bytes from file open as fd into buffer. Return the number of
    bytes actually read, 0 at end of file, or -1 if file could not be read. */
int sys_read(int fd, void *buffer, unsigned size) {
    int bytes_read = 0;
    /* Pointer to point to current position in buffer */
    char *buff = (char *) buffer;

    /* Read from keyboard input */
    if (fd == STDIN_FILENO) {
        while ((unsigned) bytes_read < size) {
            *buff = input_getc();
            buff++;
            bytes_read++;
        }
    } else if (is_valid_fd(fd)) {
        struct thread *cur = thread_current();
        struct file *open_file = get_fd(cur, fd);
        sema_down(filesys_lock);
        bytes_read = file_read(open_file, buffer, size);
        sema_up(filesys_lock);
    } else {
        bytes_read = ERR;
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
int sys_write(int fd, const void *buffer, unsigned size) {
    int bytes_written = 0;

    /* Write to console */
    if (fd == STDOUT_FILENO) {
        size_t block_size = MAX_BUF_WRI;

        /* If size greater than several hundred bytes, break up */
        while (size > bytes_written + block_size) {
            putbuf((char *)(buffer + bytes_written), block_size);
            bytes_written += block_size;
        }

        /* Write remaining bytes */
        putbuf(buffer + bytes_written, size - bytes_written);
        bytes_written = size;
    } else if (is_valid_fd(fd)) {
        struct thread *cur = thread_current();
        struct file *open_file = get_fd(cur, fd);
        sema_down(filesys_lock);
        bytes_written = file_write(open_file, buffer, size);
        sema_up(filesys_lock);
    }
    return bytes_written;
}

/*! Changes next byte to be read or written in open file *fd* to *position*,
    expressed in bytes from beginning of file. */
void sys_seek(int fd, unsigned position) {
    struct thread *cur = thread_current();
    struct file *open_file = get_fd(cur, fd);
    sema_down(filesys_lock);
    file_seek(open_file, position);
    sema_up(filesys_lock);
}


/*! Return position of next byte to be read or written in open file fd,
    expressed in bytes from beginning of file. */
unsigned sys_tell(int fd) {
    struct thread *cur = thread_current();
    struct file *open_file = get_fd(cur, fd);
    sema_down(filesys_lock);
    unsigned position = file_tell(open_file);
    sema_up(filesys_lock);
    return position;
}

/*! Close file descriptor fd. */
void sys_close(int fd) {
    struct thread *cur = thread_current();
    struct file *open_file = get_fd(cur, fd);
    sema_down(filesys_lock);
    file_close(open_file);
    sema_up(filesys_lock);
    close_fd(cur, fd);
}

/* Returns true if addr is valid for reading */
static bool valid_read_addr(const void *addr) {
    /* Check that address is below PHYS_BASE
       and then attempt to read a byte at the address */
    return addr != NULL && is_user_vaddr(addr) && (get_user(addr) != -1);
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
    Returns true if successful, false if a segfault occurred. */
static bool put_user (uint8_t *udst, uint8_t byte) {
    int error_code;
    asm ("movl $1f, %0; movb %b2, %1; 1:"
         : "=&a" (error_code), "=m" (*udst) : "q" (byte));
    return error_code != -1;
}
