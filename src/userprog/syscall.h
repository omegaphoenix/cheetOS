#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#define MAX_BUF_WRI 300
#define ARG_SIZE 4
#define ERR -1

struct semaphore filesys_lock;

void syscall_init(void);
void sys_exit(int status);

#endif /* userprog/syscall.h */

