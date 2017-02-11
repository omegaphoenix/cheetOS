#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler(struct intr_frame *);
void sys_halt(void);
void sys_exit(int status);
int sys_write(int fd, const void *buffer, unsigned size);

void syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler(struct intr_frame *f UNUSED) {
    printf("system call!\n");
    thread_exit();
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
		bytes actually written, which may be less than size if some bytes could
		not be written.
		Writing past end-of-file would normally extend the file, but file growth
		is not implemented by the basic file system. The expected behavior is to
		write as many bytes as possible up to end-of-file and return the actual
		number written, or 0 if no bytes could be written at all.
		Fd 1 writes to the console. Your code to write to the console should write
		all of buffer in one call to putbuf(), at least as long as size is not
		bigger than a few hundred bytes. (It is reasonable to break up larger
		buffers.) Otherwise, lines of text output by different processes may end
		up interleaved on the console, confusing both human readers and our
		grading scripts. */
int sys_write(int fd, const void *buffer, unsigned size) {
		if (fd == STDIN_FILENO) {
        putbuf(buffer, size);
		}
    return 0;
}
