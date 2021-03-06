#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#define ARG_SIZE 4

#include <stdbool.h>
#include "threads/thread.h"

bool install_page(void *upage, void *kpage, bool writable); 
tid_t process_execute(const char *args);
int process_wait(tid_t);
void process_exit(void);
void process_activate(void);

#endif /* userprog/process.h */

