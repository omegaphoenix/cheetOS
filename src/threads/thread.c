#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "devices/timer.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "threads/fixed_point.h"
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "userprog/syscall.h"
#ifdef USERPROG
#include "userprog/process.h"
#endif
#ifdef VM
#include "vm/page.h"
#endif
/*! Random value for struct thread's `magic' member.
    Used to detect stack overflow.  See the big comment at the top
    of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b

/*! List of processes in THREAD_READY state, that is, processes
    that are ready to run but not actually running. */
static struct list ready_list;

/*! List of all processes.  Processes are added to this list
    when they are first scheduled and removed when they exit. */
static struct list all_list;

/*! List of sleeping processes. Processes are added to this list
    when timer_sleep() is called and removed when they are woken. */
static struct list sleep_list;

/*! Idle thread. */
static struct thread *idle_thread;

/*! Lock used by allocate_tid(). */
static struct lock tid_lock;

/*! Stack frame for kernel_thread(). */
struct kernel_thread_frame {
    void *eip;                  /*!< Return address. */
    thread_func *function;      /*!< Function to call. */
    void *aux;                  /*!< Auxiliary data for function. */
};

/* Statistics. */
static long long idle_ticks;    /*!< # of timer ticks spent idle. */
static long long kernel_ticks;  /*!< # of timer ticks in kernel threads. */
static long long user_ticks;    /*!< # of timer ticks in user programs. */
static int load_avg;            /*!< System load average, estimating average
                                     number of threads run in next minute */

/* Scheduling. */
#define TIME_SLICE 4            /*!< # of timer ticks to give each thread. */
static unsigned thread_ticks;   /*!< # of timer ticks since last yield. */

/*! If false (default), use round-robin scheduler.
    If true, use multi-level feedback queue scheduler.
    Controlled by kernel command-line option "-o mlfqs". */
bool thread_mlfqs;

static void kernel_thread(thread_func *, void *aux);

static void idle(void *aux UNUSED);
static struct thread *running_thread(void);
static struct thread *next_thread_to_run(void);
static void init_thread(struct thread *, const char *name, int priority);
static void *alloc_frame(struct thread *, size_t size);
static void schedule(void);
void thread_schedule_tail(struct thread *prev);
static tid_t allocate_tid(void);

/*! Iterate through sleep_list and decrement sleep_counters.
    If a counter has reached 0, wake the thread
    and remove from sleep_list. */
void sleep_threads() {
    ASSERT(intr_get_level() == INTR_OFF);

    struct list_elem *e = list_begin(&sleep_list);

    while (e != list_end(&sleep_list)) {
        struct thread *t = list_entry(e, struct thread, sleep_elem);

        /* Decrement sleep counter and wake thread */
        if (t->sleep_counter <= 1) {
            t->sleep_counter = 0; /* reset sleep_counter */
            e = list_remove(e); /* remove thread from list */
            thread_unblock(t);
        }
        else {
            t->sleep_counter--;
            e = list_next(e);
        }
    }
}

/* Add thread to sleep_list. */
void add_sleep_thread(struct thread *t) {
	list_push_back(&sleep_list, &t->sleep_elem);
}

/*! Returns the initial thread. This allows us to verify that a thread is not
    the initial thread before freeing. */
struct thread *get_initial_thread(void) {
    return initial_thread;
}

/*! Returns the child thread with this tid. */
struct thread *get_child_thread(tid_t child_tid) {
    struct thread *cur = thread_current();
    struct list_elem *e;
    for (e = list_begin(&cur->kids); e != list_end(&cur->kids);
         e = list_next(e)) {
        struct thread *kid = list_entry(e, struct thread, kid_elem);
        if (kid->tid == child_tid) {
            return kid;
        }
    }
    return NULL;
}

/*! Initializes the threading system by transforming the code
    that's currently running into a thread.  This can't work in
    general and it is possible in this case only because loader.S
    was careful to put the bottom of the stack at a page boundary.

    Also initializes the run queue and the tid lock.

    After calling this function, be sure to initialize the page allocator
    before trying to create any threads with thread_create().

    It is not safe to call thread_current() until this function finishes. */
void thread_init(void) {
    ASSERT(intr_get_level() == INTR_OFF);

    lock_init(&tid_lock);
    list_init(&ready_list);
    list_init(&all_list);
    list_init(&sleep_list);

    /* System boot, load_avg starts at 0 */
    load_avg = 0;

    /* Set up a thread structure for the running thread. */
    initial_thread = running_thread();
    init_thread(initial_thread, "main", PRI_DEFAULT);
    initial_thread->status = THREAD_RUNNING;
    initial_thread->tid = allocate_tid();
}

/*! Starts preemptive thread scheduling by enabling interrupts.
    Also creates the idle thread. */
void thread_start(void) {
    /* Create the idle thread. */
    struct semaphore idle_started;
    sema_init(&idle_started, 0);
    thread_create("idle", PRI_MIN, idle, &idle_started);

    /* Start preemptive thread scheduling. */
    intr_enable();

    /* Wait for the idle thread to initialize idle_thread. */
    sema_down(&idle_started);
}

/*! Called by the timer interrupt handler at each timer tick.
    Thus, this function runs in an external interrupt context. */
void thread_tick(void) {
    struct thread *t = thread_current();
    int num_ready_threads = list_size(&ready_list);

    /* Update statistics. */
    if (t == idle_thread)
        idle_ticks++;
#ifdef USERPROG
    else if (t->pagedir != NULL)
        user_ticks++;
#endif
    else {
        kernel_ticks++;
        num_ready_threads = list_size(&ready_list) + 1;
    }

    /* Update cpu_usage */
    t->recent_cpu += FIXED_ONE;

    /* Update load balance and cpu_usage every second */
    if (thread_mlfqs && timer_ticks() % TIMER_FREQ == 0 && !list_empty(&all_list)) {
        struct list_elem *cpu_e;
        load_avg = calculate_load_avg(load_avg, num_ready_threads);
        for (cpu_e = list_begin(&all_list); cpu_e != list_end(&all_list);
             cpu_e = list_next(cpu_e)) {

            struct thread *new_t = list_entry(cpu_e, struct thread, allelem);
            new_t->recent_cpu =
                calculate_cpu_usage(new_t->recent_cpu, load_avg, new_t->niceness);
        }
    }

    /* Every fourth tick, update priorities */
    if (thread_mlfqs && timer_ticks() % 4 == 0 && !list_empty(&all_list)) {
        struct list_elem *prio_e;
        for (prio_e = list_begin(&all_list); prio_e != list_end(&all_list);
             prio_e = list_next(prio_e)) {
            struct thread *new_t = list_entry(prio_e, struct thread, allelem);
            ASSERT(is_thread(new_t));

            new_t->priority = calculate_priority(new_t->recent_cpu, new_t->niceness);
        }

    }

    /* Decrement sleep counter for all threads. */
	sleep_threads();

    /* Enforce preemption. */
    if (++thread_ticks >= TIME_SLICE)
        intr_yield_on_return();
}

/*! Prints thread statistics. */
void thread_print_stats(void) {
    printf("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
           idle_ticks, kernel_ticks, user_ticks);
}

/*! Creates a new kernel thread named NAME with the given initial PRIORITY,
    which executes FUNCTION passing AUX as the argument, and adds it to the
    ready queue.  Returns the thread identifier for the new thread, or
    TID_ERROR if creation fails.

    If thread_start() has been called, then the new thread may be scheduled
    before thread_create() returns.  It could even exit before thread_create()
    returns.  Contrariwise, the original thread may run for any amount of time
    before the new thread is scheduled.  Use a semaphore or some other form of
    synchronization if you need to ensure ordering.

    The code provided sets the new thread's `priority' member to PRIORITY, but
    no actual priority scheduling is implemented.  Priority scheduling is the
    goal of Problem 1-3. */
tid_t thread_create(const char *name, int priority, thread_func *function,
                    void *aux) {
    struct thread *t;
    struct kernel_thread_frame *kf;
    struct switch_entry_frame *ef;
    struct switch_threads_frame *sf;
    tid_t tid;

    ASSERT(function != NULL);

    /* Allocate thread. */
    t = palloc_get_page(PAL_ZERO);
    if (t == NULL)
        return TID_ERROR;

    /* Initialize thread. */
    init_thread(t, name, priority);
    tid = t->tid = allocate_tid();

    /* Stack frame for kernel_thread(). */
    kf = alloc_frame(t, sizeof *kf);
    kf->eip = NULL;
    kf->function = function;
    kf->aux = aux;

    /* Stack frame for switch_entry(). */
    ef = alloc_frame(t, sizeof *ef);
    ef->eip = (void (*) (void)) kernel_thread;

    /* Stack frame for switch_threads(). */
    sf = alloc_frame(t, sizeof *sf);
    sf->eip = switch_entry;
    sf->ebp = 0;

    /* Add new thread to parent's (current thread) list */
    struct thread *parent = thread_current();
    list_push_back(&parent->kids, &t->kid_elem);
    t->parent = parent;

    /* Set current directory to parent's current directory */
    t->cur_dir_inode = parent->cur_dir_inode;
    if (t->cur_dir_inode != NULL) {
        inc_in_use(t->cur_dir_inode);
    }


    ASSERT(t->done_sema.value == 1);
    sema_down(&t->done_sema);

    /* Add to run queue. */
    int prev_highest_priority = get_highest_priority();
    thread_unblock(t);

    /* Yield current thread if higher priority thread was added */
    if (prev_highest_priority < t->priority) {
        thread_yield();
    }

    return tid;
}

/*! Puts the current thread to sleep.  It will not be scheduled
    again until awoken by thread_unblock().

    This function must be called with interrupts turned off.  It is usually a
    better idea to use one of the synchronization primitives in synch.h. */
void thread_block(void) {
    ASSERT(!intr_context());
    ASSERT(intr_get_level() == INTR_OFF);

    thread_current()->status = THREAD_BLOCKED;
    schedule();
}

/*! Transitions a blocked thread T to the ready-to-run state.  This is an
    error if T is not blocked.  (Use thread_yield() to make the running
    thread ready.)

    This function does not preempt the running thread.  This can be important:
    if the caller had disabled interrupts itself, it may expect that it can
    atomically unblock a thread and update other data. */
void thread_unblock(struct thread *t) {
    ASSERT(is_thread(t));

    enum intr_level old_level = intr_disable();
    ASSERT(t->status == THREAD_BLOCKED);
    ASSERT(is_thread(t));
    list_push_back(&ready_list, &t->elem);
    t->status = THREAD_READY;
    intr_set_level(old_level);
}

/*! Returns the name of the running thread. */
const char * thread_name(void) {
    return thread_current()->name;
}

/*! Returns the running thread.
    This is running_thread() plus a couple of sanity checks.
    See the big comment at the top of thread.h for details. */
struct thread * thread_current(void) {
    struct thread *t = running_thread();
    /* Make sure T is really a thread.
       If either of these assertions fire, then your thread may
       have overflowed its stack.  Each thread has less than 4 kB
       of stack, so a few big automatic arrays or moderate
       recursion can cause stack overflow. */
    ASSERT(is_thread(t));
    ASSERT(t->status == THREAD_RUNNING);

    return t;
}

/*! Returns the running thread's tid. */
tid_t thread_tid(void) {
    return thread_current()->tid;
}

/*! Deschedules the current thread and destroys it.  Never
    returns to the caller. */
void thread_exit(void) {
    ASSERT(!intr_context());


    /* Remove thread from all threads list, set our status to dying,
       and schedule another process.  That process will destroy us
       when it calls thread_schedule_tail(). */
    struct thread *cur = thread_current();
    struct list_elem *e;

    /* Tell blocking lock we are no longer waiting for it. */
    if (cur->blocking_lock != NULL) {
        list_remove(&cur->lock_elem);
    }

    /* Free all locks. */
    while (!list_empty(&cur->locks_acquired)) {
        e = list_begin(&cur->locks_acquired);
        struct lock *lock = list_entry(e, struct lock, elem);
        lock_release(lock);
    }

    /* All file buffers should be freed in sys_exit. */
    ASSERT (list_empty(&cur->open_files));

#ifdef VM
    /* All mappings should be freed in sys_exit too. */
    ASSERT (list_empty(&cur->mappings));
#endif

    /* Let kids know that parent is dead so that their page is freed without
       waiting for the parent. Will be freed in thread_schedule_tail().*/

    while (!list_empty(&cur->kids)) {
        e = list_pop_front(&cur->kids);
        struct thread *kid = list_entry(e, struct thread, kid_elem);
        kid->parent = NULL;
        sema_up(&kid->done_sema);
    }

#ifdef CACHE
    /* Close current directory */
    if (cur->cur_dir_inode != NULL) { // maybe assert
        inode_close(cur->cur_dir_inode);
    }
#endif

#ifdef USERPROG
    process_exit();
#endif

    if (cur->parent != NULL) {
        list_remove(&cur->kid_elem);
    }

    intr_disable();
    list_remove(&cur->allelem);
    cur->status = THREAD_DYING;
    schedule();
    NOT_REACHED();
}

/*! Yields the CPU.  The current thread is not put to sleep and
    may be scheduled again immediately at the scheduler's whim. */
void thread_yield(void) {
    struct thread *cur = thread_current();
    enum intr_level old_level;

    ASSERT(!intr_context());

    old_level = intr_disable();
    if (cur != idle_thread) {
        ASSERT(is_thread(cur));
        list_push_back(&ready_list, &cur->elem);
    }
    cur->status = THREAD_READY;
    schedule();
    intr_set_level(old_level);
}

/*! Invoke function 'func' on all threads, passing along 'aux'.
    This function must be called with interrupts off. */
void thread_foreach(thread_action_func *func, void *aux) {
    struct list_elem *e;

    ASSERT(intr_get_level() == INTR_OFF);

    for (e = list_begin(&all_list); e != list_end(&all_list);
         e = list_next(e)) {
        struct thread *t = list_entry(e, struct thread, allelem);
        func(t, aux);
    }
}

/*! Sets the current thread's priority to NEW_PRIORITY. */
void thread_set_priority(int new_priority) {
    if (!thread_mlfqs) {
        thread_current()->priority = new_priority;
        /* Yield current thread if it is no longer the highest priority */
        if (!is_highest_priority(new_priority)) {
            thread_yield();
        }
    }
}

/*! Sets the RECIPIENT thread's donated priority to NEW_PRIORITY. */
void thread_donate_priority(struct thread *recipient, int new_priority) {
    ASSERT(is_thread(recipient));
    if (recipient->donated_priority < new_priority) {
        recipient->donated_priority = new_priority;

        /* Chain the donate if it changes*/
        if (recipient->blocking_lock != NULL) {
            struct thread *blocker = recipient->blocking_lock->holder;
            if (blocker != NULL) {
                ASSERT(is_thread(blocker));
                thread_donate_priority(blocker, new_priority);
            }
        }
    }
    /* Yield current thread if it is no longer the highest priority */
    if (!is_highest_priority(thread_get_priority())) {
        thread_yield();
    }
}

/*! Resets the RECIPIENT thread's donated priority based on locks_acquired. */
void thread_reset_priority(struct thread *recipient) {
    recipient->donated_priority = PRI_MIN;
    /* Iterate through locks to set donated_priority */
    if (!list_empty(&recipient->locks_acquired)) {
        struct list_elem *e;
        for (e = list_begin(&recipient->locks_acquired);
                e != list_end(&recipient->locks_acquired);
                e = list_next(e)) {
            struct lock *cur_lock = list_entry(e, struct lock, elem);
            int donated_lock_priority = calc_lock_priority(cur_lock);
            if (donated_lock_priority > recipient->donated_priority) {
                recipient->donated_priority = donated_lock_priority;
            }
        }
    }
}

/*! Returns the priority of the given thread */
int get_priority(struct thread *thread_to_check) {
    ASSERT(is_thread(thread_to_check));
    int donated_priority = thread_to_check->donated_priority;
    int own_priority = thread_to_check->priority;

    if (donated_priority > own_priority) {
        return donated_priority;
    }
    else {
        return own_priority;
    }
}

/*! Returns the current thread's priority. */
int thread_get_priority(void) {
    struct thread *cur = thread_current();
    return get_priority(cur);
}

/*! Sets the current thread's nice value to NICE. */
void thread_set_nice(int nice) {

    struct thread *t = thread_current();

    t->niceness = nice;
    t->priority = calculate_priority(t->recent_cpu, t->niceness);
    if (!is_highest_priority(t->priority)) {
        thread_yield();
    }
}

/*! Returns the current thread's nice value. */
int thread_get_nice(void) {
    return thread_current()->niceness;
}

/*! Returns 100 times the system load average. */
int thread_get_load_avg(void) {
    int fixed_load_avg = 100 * load_avg;
    return convert_to_integer_round_nearest(fixed_load_avg, FIXED_POINT_Q);
}

/*! Returns 100 times the current thread's recent_cpu value. */
int thread_get_recent_cpu(void) {
    /* Not yet implemented. */
    int fixed_cpu_usage = 100 * thread_current()->recent_cpu;
    return convert_to_integer_round_nearest(fixed_cpu_usage, FIXED_POINT_Q);
}

/*! Returns priority of highest priority thread including argument. */
int highest_priority(int priority) {
    int highest_priority = get_highest_priority();
    if (highest_priority < priority) {
        return priority;
    }
    else {
        return highest_priority;
    }
}

/*! Returns priority of highest priority thread in ready_list. */
int get_highest_priority(void) {
    enum intr_level old_level = intr_disable();
    // Return minimum if no threads
    int highest_priority_val = thread_current()->priority;
    // Find max priority of threads
    struct list_elem *e;
    for (e = list_begin(&ready_list); e != list_end(&ready_list);
         e = list_next(e)) {
        struct thread *t = list_entry(e, struct thread, elem);
        ASSERT(is_thread(t));
        int curr_priority = get_priority(t);

        if (curr_priority > highest_priority_val) {
            highest_priority_val = curr_priority;
        }
    }
    intr_set_level(old_level);

    return highest_priority_val;
}

/*! Returns true if test_priority is the highest priority. */
bool is_highest_priority(int test_priority) {
    int highest = get_highest_priority();
    return test_priority >= highest;
}

/*! Return true if fd is in range. */
bool is_valid_fd(int fd) {
    return fd >= CONSOLE_FD;
}

/*! Return true if fd is in range and exists. */
bool is_existing_fd(struct thread *cur, int fd) {
    if (!is_valid_fd(fd)) {
        return false;
    }

    /* Check if fd is in open_files list. */
    struct list_elem *e;
    for (e = list_begin(&cur->open_files); e != list_end(&cur->open_files);
         e = list_next(e)) {
        struct sys_file *cur_file = list_entry(e, struct sys_file, file_elem);
        if (cur_file->fd == fd) {
            return true;
        }
    }

    /* Didn't find fd. */
    return false;
}

/*! Get next fd. */
int next_fd(struct thread *cur) {
    int fd = cur->num_files + CONSOLE_FD;
    cur->num_files++;
    ASSERT (is_valid_fd(fd) && !is_existing_fd(cur, fd));
    return fd;
}

/*! Add file to open_files array. Return -1 if fails */
int add_open_file(struct thread *cur, struct file *file, int fd) {
    /* Initialize file */
    struct sys_file *new_file = palloc_get_page(PAL_ZERO);
    /* Not enough memory. */
    if (new_file == NULL) {
#ifdef USERPROG
        file_close(file);
#endif
        return ERR;
    }
    memset(new_file, 0, sizeof *new_file);
    new_file->file = file;
    new_file->fd = fd;

    if (is_valid_fd(fd) && !is_existing_fd(cur, fd)) {
        list_push_back(&cur->open_files, &new_file->file_elem);
        return fd;
    }

    /* Couldn't find file. */
    return ERR;
}

/*! Get file with file descriptor *fd*. */
struct file *get_fd(struct thread *cur, int fd) {
    struct sys_file *cur_file;
    struct list_elem *e;
    for (e = list_begin(&cur->open_files);
         e != list_end(&cur->open_files);
         e = list_next(e)) {
        cur_file = list_entry(e, struct sys_file, file_elem);
        if (cur_file->fd == fd) {
            return cur_file->file;
        }
    }

    /* Couldn't find file. */
    return NULL;
}

/*! Close file with file descriptor *fd*. Assumes file is already closed so
    just need to remove it from the thread's list. */
void close_fd(struct thread *cur, int fd) {
    ASSERT(is_existing_fd(cur, fd));

    struct list_elem *e;
    for (e = list_begin(&cur->open_files); e != list_end(&cur->open_files);
         e = list_next(e)) {
        struct sys_file *cur_file = list_entry(e, struct sys_file, file_elem);
        if (cur_file->fd == fd) {
            list_remove(&cur_file->file_elem);
#ifdef USERPROG
            file_close(cur_file->file);
#endif
            palloc_free_page(cur_file);
            return;
        }
    }
}

#ifdef VM
/* Returns true if mapping is >= 0. -1 is for failutres. */
bool is_valid_mapping(int mapping) {
    return mapping >= 0;
}

/* Returns true if mapping ID is in use */
bool is_existing_mapping(struct thread *cur, int mapping) {
    if (!is_valid_mapping(mapping)) {
        return false;
    }

    /* Check if mapping is in mmap_files list. */
    struct list_elem *e;
    for (e = list_begin(&cur->mappings); e != list_end(&cur->mappings);
         e = list_next(e)) {
        struct mmap_file *cur_mmap = list_entry(e, struct mmap_file, mmap_elem);
        if (cur_mmap->mapping == mapping) {
            return true;
        }
    }

    /* Didn't find mapping. */
    return false;
}

/* Return next mapping ID. */
int next_mapping(struct thread *cur) {
    int mapping = cur->num_mappings;
    cur->num_mappings++;
    ASSERT (is_valid_mapping(mapping) && !is_existing_mapping(cur, mapping));
    return mapping;
}

/* Add new mapping to list. Returns mapping or -1 on failure. */
int add_mmap(struct thread *cur, void *addr, int fd, int mapping) {
    /* Initialize file */
    struct mmap_file *new_mapping = calloc(1, sizeof(struct mmap_file));
    /* Not enough memory. */
    if (new_mapping == NULL) {
        return ERR;
    }
    new_mapping->addr = addr;
    new_mapping->mapping = mapping;
    new_mapping->fd = fd;

    if (is_valid_mapping(mapping) && !is_existing_mapping(cur, mapping)) {
        list_push_back(&cur->mappings, &new_mapping->mmap_elem);
        return mapping;
    }

    /* Couldn't find page. */
    return ERR;
}

/* Returns page of mapped memory. */
struct mmap_file *get_mmap(struct thread *cur, int mapping) {
    struct list_elem *e;
    for (e = list_begin(&cur->mappings); e != list_end(&cur->mappings);
         e = list_next(e)) {
        struct mmap_file *cur_mmap = list_entry(e, struct mmap_file, mmap_elem);
        if (cur_mmap->mapping == mapping) {
            return cur_mmap;
        }
    }

    /* Couldn't find mapping. */
    return NULL;
}

/* Removes mapping from list. Assumes pages have already been written back. */
void remove_mmap(struct thread *cur, int mapping) {
    ASSERT(is_existing_mapping(cur, mapping));

    struct list_elem *e;
    for (e = list_begin(&cur->mappings); e != list_end(&cur->mappings);
         e = list_next(e)) {
        struct mmap_file *cur_mmap = list_entry(e, struct mmap_file, mmap_elem);
        if (cur_mmap->mapping == mapping) {
            list_remove(&cur_mmap->mmap_elem);
            free(cur_mmap);
            return;
        }
    }
}
#endif


/*! Idle thread.  Executes when no other thread is ready to run.

    The idle thread is initially put on the ready list by thread_start().
    It will be scheduled once initially, at which point it initializes
    idle_thread, "up"s the semaphore passed to it to enable thread_start()
    to continue, and immediately blocks.  After that, the idle thread never
    appears in the ready list.  It is returned by next_thread_to_run() as a
    special case when the ready list is empty. */
static void idle(void *idle_started_ UNUSED) {
    struct semaphore *idle_started = idle_started_;
    idle_thread = thread_current();
    sema_up(idle_started);

    for (;;) {
        /* Let someone else run. */
        intr_disable();
        thread_block();

        /* Re-enable interrupts and wait for the next one.

           The `sti' instruction disables interrupts until the completion of
           the next instruction, so these two instructions are executed
           atomically.  This atomicity is important; otherwise, an interrupt
           could be handled between re-enabling interrupts and waiting for the
           next one to occur, wasting as much as one clock tick worth of time.

           See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
           7.11.1 "HLT Instruction". */
        asm volatile ("sti; hlt" : : : "memory");
    }
}

/*! Function used as the basis for a kernel thread. */
static void kernel_thread(thread_func *function, void *aux) {
    ASSERT(function != NULL);

    intr_enable();       /* The scheduler runs with interrupts off. */
    function(aux);       /* Execute the thread function. */
    thread_exit();       /* If function() returns, kill the thread. */
}

/*! Returns the running thread. */
struct thread * running_thread(void) {
    uint32_t *esp;

    /* Copy the CPU's stack pointer into `esp', and then round that
       down to the start of a page.  Because `struct thread' is
       always at the beginning of a page and the stack pointer is
       somewhere in the middle, this locates the curent thread. */
    asm ("mov %%esp, %0" : "=g" (esp));
    return pg_round_down(esp);
}

/*! Returns true if T appears to point to a valid thread. */
bool is_thread(struct thread *t) {
    return t != NULL && t->magic == THREAD_MAGIC;
}

/*! Does basic initialization of T as a blocked thread named NAME. */
static void init_thread(struct thread *t, const char *name, int priority) {
    enum intr_level old_level;

    ASSERT(t != NULL);
    ASSERT(PRI_MIN <= priority && priority <= PRI_MAX);
    ASSERT(name != NULL);

    memset(t, 0, sizeof *t);
    t->status = THREAD_BLOCKED;
    strlcpy(t->name, name, sizeof t->name);
    t->stack = (uint8_t *) t + PGSIZE;
    t->donated_priority = PRI_MIN;
    t->exit_status = 0;
    t->magic = THREAD_MAGIC;
    t->sleep_counter = 0; /* set to 0 if thread is not sleeping */
    list_init(&t->locks_acquired);
    list_init(&t->kids);
    list_init(&t->open_files);
    /* Block process_wait of parent until this process is ready to die. */
    sema_init(&t->wait_sema, 0);
    sema_init(&t->done_sema, 1);
    lock_init(&t->wait_lock);
    t->loaded = false;
    t->waited_on = false;
    t->num_files = 0;
    t->parent = NULL;

    if (list_empty(&all_list)) {
        t->niceness = 0;  /* Set niceness to 0 on initial thread */
        t->recent_cpu = 0; /* Set cpu_usage to 0 on initial thread */
    }
    else {
        /* Inherit niceness and cpu_usage from parent */
        t->niceness = thread_current()->niceness;
        t->recent_cpu = thread_current()->recent_cpu;
    }

    /* Priority setting */
    if (thread_mlfqs) {
        t->priority = calculate_priority(t->recent_cpu, t->niceness);
    }
    else {
        t->priority = priority;
    }

#ifdef VM
    list_init(&t->mappings);
    t->num_mappings = 0;
#endif
#ifdef CACHE
    t->cur_dir_inode = NULL;
#endif

    old_level = intr_disable();
    list_push_back(&all_list, &t->allelem);
    intr_set_level(old_level);
}

/*! Allocates a SIZE-byte frame at the top of thread T's stack and
    returns a pointer to the frame's base. */
static void * alloc_frame(struct thread *t, size_t size) {
    /* Stack data is always allocated in word-size units. */
    ASSERT(is_thread(t));
    ASSERT(size % sizeof(uint32_t) == 0);

    t->stack -= size;
    return t->stack;
}

/*! Chooses and returns the next thread to be scheduled.  Should return a
    thread from the run queue, unless the run queue is empty.  (If the running
    thread can continue running, then it will be in the run queue.)  If the
    run queue is empty, return idle_thread. */
static struct thread * next_thread_to_run(void) {
    ASSERT(intr_get_level() == INTR_OFF);
    if (list_empty(&ready_list)) {
        return idle_thread;
    }
    else {
        struct list_elem *next = list_begin(&ready_list);
        struct thread *t = list_entry(next, struct thread, elem);
        int highest_priority_val = get_priority(t);

        // Find max priority of threads
        struct list_elem *e;
        struct thread *ret;
        for (e = list_next(next); e != list_end(&ready_list);
                e = list_next(e)) {
            t = list_entry(e, struct thread, elem);
            int curr_priority = get_priority(t);

            if (curr_priority > highest_priority_val) {
                highest_priority_val = curr_priority;
                next = e;
            }
        }
        ret = list_entry(next, struct  thread, elem);
        list_remove(next);
        return ret;
    }
}

/*! Completes a thread switch by activating the new thread's page tables, and,
    if the previous thread is dying, destroying it.

    At this function's invocation, we just switched from thread PREV, the new
    thread is already running, and interrupts are still disabled.  This
    function is normally invoked by thread_schedule() as its final action
    before returning, but the first time a thread is scheduled it is called by
    switch_entry() (see switch.S).

    It's not safe to call printf() until the thread switch is complete.  In
    practice that means that printf()s should be added at the end of the
    function.

    After this function and its caller returns, the thread switch is
    complete. */
void thread_schedule_tail(struct thread *prev) {
    struct thread *cur = running_thread();

    ASSERT(intr_get_level() == INTR_OFF);

    /* Mark us as running. */
    cur->status = THREAD_RUNNING;

    /* Start new time slice. */
    thread_ticks = 0;

#ifdef USERPROG
    /* Activate the new address space. */
    process_activate();
#endif

    /* If the thread we switched from is dying, destroy its struct thread.
       This must happen late so that thread_exit() doesn't pull out the rug
       under itself.  (We don't free initial_thread because its memory was
       not obtained via palloc().) */
    if (prev != NULL && prev->status == THREAD_DYING &&
        prev != initial_thread) {
        ASSERT(prev != cur);

        /* All these elements should have been removed from their lists. */
        ASSERT(try_remove(&prev->allelem) == NULL);
        ASSERT(try_remove(&prev->sleep_elem) == NULL);
        ASSERT(try_remove(&prev->elem) == NULL);
        ASSERT(try_remove(&prev->lock_elem) == NULL);
        ASSERT(try_remove(&prev->kid_elem) == NULL);

        /* All lists should be emptied. */
        ASSERT(list_empty(&prev->locks_acquired));
        ASSERT(list_empty(&prev->open_files));
        ASSERT(list_empty(&prev->kids));

        /* Free thread memory. */
        palloc_free_page(prev);
    }
}

/*! Schedules a new process.  At entry, interrupts must be off and the running
    process's state must have been changed from running to some other state.
    This function finds another thread to run and switches to it.

    It's not safe to call printf() until thread_schedule_tail() has
    completed. */
static void schedule(void) {
    struct thread *cur = running_thread();
    struct thread *next = next_thread_to_run();
    struct thread *prev = NULL;

    ASSERT(intr_get_level() == INTR_OFF);
    ASSERT(cur->status != THREAD_RUNNING);
    ASSERT(is_thread(next));

    if (cur != next)
        prev = switch_threads(cur, next);
    thread_schedule_tail(prev);
}

/*! Returns a tid to use for a new thread. */
static tid_t allocate_tid(void) {
    static tid_t next_tid = 1;
    tid_t tid;

    lock_acquire(&tid_lock);
    tid = next_tid++;
    lock_release(&tid_lock);

    return tid;
}

/*! Offset of `stack' member within `struct thread'.
    Used by switch.S, which can't figure it out on its own. */
uint32_t thread_stack_ofs = offsetof(struct thread, stack);

