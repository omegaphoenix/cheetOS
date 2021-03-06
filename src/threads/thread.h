/*! \file thread.h
 *
 * Declarations for the kernel threading functionality in PintOS.
 */

#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <hash.h>
#include <list.h>
#include <stdint.h>
#include "synch.h"

/* Open file. This is for a linked list of open files in each thread. */
struct sys_file {
    struct file *file;
    int fd;
    struct list_elem file_elem;
};

/* Memory mapped files. This is for a linked list of mmapped files in each thread. */
struct mmap_file {
    void *addr;         /*!< Virtual address to which file is mapped. */
    int fd;             /*!< Mapped file. */
    int mapping;        /*!< Unique mmap ID. */
    struct list_elem mmap_elem;
};

/*! Initial thread, the thread running init.c:main(). */
struct thread *initial_thread;

/*! States in a thread's life cycle. */
enum thread_status {
    THREAD_RUNNING,     /*!< Running thread. */
    THREAD_READY,       /*!< Not running but ready to run. */
    THREAD_BLOCKED,     /*!< Waiting for an event to trigger. */
    THREAD_DYING        /*!< About to be destroyed. */
};

/*! Thread identifier type.
    You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /*!< Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /*!< Lowest priority. */
#define PRI_DEFAULT 31                  /*!< Default priority. */
#define PRI_MAX 63                      /*!< Highest priority. */

/* Open files' file descriptors. */
#define CONSOLE_FD 2                    /*!< fd 0 and 1 reserved. */

/*! A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

\verbatim
        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
             |               tid               |
        0 kB +---------------------------------+
\endverbatim

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion.

   The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list.
*/
struct thread {
    /*! Owned by thread.c. */
    /**@{*/
    tid_t tid;                          /*!< Thread identifier. */
    enum thread_status status;          /*!< Thread state. */
    char name[16];                      /*!< Name (for debugging purposes). */
    uint8_t *stack;                     /*!< Saved stack pointer. */
    int priority;                       /*!< Priority. */
    int donated_priority;               /*!< Donated priority. */
    struct list_elem allelem;           /*!< List element for all threads list. */
    struct list_elem sleep_elem;        /*!< List element for sleeping list. */
    int64_t sleep_counter;              /*!< Number of ticks left to sleep. */
    int niceness;                       /*!< Niceness value for BSD CPU priority. */
    int recent_cpu;                     /*!< Most recent CPU time usage. Fixed point. */
    struct lock *blocking_lock;         /*!< Lock that is blocking this thread */
    struct list locks_acquired;         /*!< Locks this thread is blocking */
    /**@}*/

    /*! Shared between thread.c and synch.c. */
    /**@{*/
    struct list_elem elem;              /*!< List element. */
    struct list_elem lock_elem;         /*!< List element for lock's blocked_threads. */
    /**@}*/

    /*! Shared between thread.c and userprog/syscall.c. */
    /**@{*/
    int num_files;                      /*!< Number of open files (counting closed ones). */
    struct list open_files;             /*!< Open files. */
    /**@}*/

    /*! Shared between userprog/process.c and thread.c. */
    /**@{*/
    int exit_status;                    /*!< Exit status to be retrieved by parent. */
    struct list kids;                   /*!< List of children processes. */
    struct list_elem kid_elem;          /*!< List element for parent's kids list. */
    struct semaphore wait_sema;         /*!< Sempahore for process_wait. */
    struct semaphore done_sema;         /*!< Sempahore for process_exit. */
    bool waited_on;                     /*!< True if process_wait has been called. */
    struct lock wait_lock;              /*!< Lock for checking waited_on. */
    /**@}*/

    /*! Shared between by userprog/process.c and userprog.syscall.c and
        thread.c. */
    /**@{*/
    struct thread *parent;              /*!< Thread that created this one. */
    struct semaphore exec_load;         /*!< Semaphore for checking when executable has loaded. */
    bool loaded;                        /*!< Check if exec loaded successfully. */
    struct file *executable;            /*!< Executable to keep open until done. */
    /**@}*/

#ifdef USERPROG
    /*! Owned by userprog/process.c. */
    /**@{*/
    uint32_t *pagedir;                  /*!< Page directory. */
    /**@}*/
#endif

#ifdef VM
    struct hash sup_page;              /*!< Supplemental Page Table. */
    struct list mappings;              /*!< Memory mapped files. */
    int num_mappings;                  /*!< Number of mappings (including unmapped). */
    uint8_t *esp;                      /*!< esp to pass to page_fault */
#endif

#ifdef CACHE
    struct inode *cur_dir_inode;               /*!< Current directory inode. */
#endif

    /*! Owned by thread.c. */
    /**@{*/
    unsigned magic;                     /* Detects stack overflow. */
    /**@}*/
};

/*! If false (default), use priority scheduler.
    If true, use multi-level feedback queue scheduler.
    Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

bool is_thread(struct thread *);
void thread_init(void);
void thread_start(void);

void thread_tick(void);
void thread_print_stats(void);

typedef void thread_func(void *aux);
tid_t thread_create(const char *name, int priority, thread_func *, void *);

void thread_block(void);
void thread_unblock(struct thread *);

struct thread *thread_current (void);
tid_t thread_tid(void);
const char *thread_name(void);

void thread_exit(void) NO_RETURN;
void thread_yield(void);

/*! Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func(struct thread *t, void *aux);

void thread_foreach(thread_action_func *, void *);

int get_priority(struct thread *thread_to_check);
int thread_get_priority(void);
void thread_set_priority(int);
void thread_donate_priority(struct thread *recipient, int new_priority);
void thread_reset_priority(struct thread *recipient);

int thread_get_nice(void);
void thread_set_nice(int);
int thread_get_recent_cpu(void);
int thread_get_load_avg(void);

int highest_priority(int priority);
int get_highest_priority(void);
bool is_highest_priority(int test_priority);

bool is_valid_fd(int fd);
bool is_existing_fd(struct thread *cur, int fd);
int next_fd(struct thread *cur);
int add_open_file(struct thread *cur, struct file *file, int fd);
struct file *get_fd(struct thread *cur, int fd);
void close_fd(struct thread *cur, int fd);

#ifdef VM
bool is_valid_mapping(int mapping);
bool is_existing_mapping(struct thread *cur, int mapping);
int next_mapping(struct thread *cur);
int add_mmap(struct thread *cur, void *addr, int fd, int mapping);
struct mmap_file *get_mmap(struct thread *cur, int mapping);
void remove_mmap(struct thread *cur, int mapping);
#endif

void add_sleep_thread(struct thread *);
void sleep_threads(void);

struct thread *get_initial_thread(void);
struct thread *get_child_thread(tid_t child_tid);

#endif /* threads/thread.h */

