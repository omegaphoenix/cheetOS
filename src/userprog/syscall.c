#include "userprog/syscall.h"

#include <stdio.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler(struct intr_frame *);

/* System calls */
void sys_halt(void);
void sys_exit(int status);
int sys_write(int fd, const void *buffer, unsigned size);

/* User memory access */
static int get_user(const uint8_t *uaddr);
static bool put_user (uint8_t *udst, uint8_t byteput);
static bool valid_read_addr(const void *addr) UNUSED;
static bool valid_write_addr(void *addr) UNUSED;

void syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler(struct intr_frame *f UNUSED) {
    int *fd;
    void *buffer;
    unsigned int *size;
    printf("system call!\n");
    /* Get the system call number */
    if (f == NULL || !valid_read_addr(f->esp)) {
        sys_exit(-1);
        return;
    }
    int syscall_no = *((int *) f->esp);

    switch (syscall_no) {
        case SYS_HALT:
            sys_halt();
            break;
        case SYS_EXIT:
            sys_exit(0);
            break;
        case SYS_WRITE:
            fd = ((int *) (f->esp + ARG_SIZE));
            buffer = f->esp + ARG_SIZE * 2;
            size = (unsigned int *) (f->esp + ARG_SIZE * 3);
            if (!valid_read_addr(fd)
                    || !valid_read_addr(buffer)
                    || !valid_read_addr(size)) {
                sys_exit(-1);
                return;
            }
            f->eax = sys_write(*fd, buffer, *size);
            break;
        default:
            printf("Unimplemented system call number\n");
            sys_exit(-1);
            break;
    }
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
    if (fd == STDIN_FILENO) {
        size_t block_size = MAX_BUF_WRI;

        /* If size greater than several hundred bytes, break up */
        while (size > bytes_written + block_size) {
            putbuf((char *)(buffer + bytes_written), block_size);
            bytes_written += block_size;
        }

        /* Write remaining bytes */
        putbuf((char *)(buffer + bytes_written), size - bytes_written);
        bytes_written = size;
    }
    return bytes_written;
}

/* Returns true if addr is valid for reading */
static bool valid_read_addr(const void *addr) {
    /* Check that address is below PHYS_BASE
       and then attempt to read a byte at the address */
    return addr != NULL && is_user_vaddr(addr) && get_user(addr);
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
