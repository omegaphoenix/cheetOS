#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#define MAX_BUF_WRI 300
#define ARG_SIZE 4
#define ERR -1
#define NUM_ENTRIES 10 /* Temporarily fix number of entries in new directories */

void syscall_init(void);
void sys_exit(int status);
void acquire_file_lock(void);
void release_file_lock(void);

#endif /* userprog/syscall.h */

