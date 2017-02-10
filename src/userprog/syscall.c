#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler(struct intr_frame *);
void sys_halt(void);
void sys_exit(int status);

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
