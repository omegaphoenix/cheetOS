/*! \file synch.h
 *
 * Data structures and function declarations for thread synchronization
 * primitives.
 */

#ifndef THREADS_SYNCH_H
#define THREADS_SYNCH_H

#include <list.h>
#include <stdbool.h>

/*! A counting semaphore. */
struct semaphore {
    unsigned value;             /*!< Current value. */
    struct list waiters;        /*!< List of waiting threads. */
};

void sema_init(struct semaphore *, unsigned value);
void sema_down(struct semaphore *);
bool sema_try_down(struct semaphore *);
void sema_up(struct semaphore *);
void sema_self_test(void);

/*! Lock. */
struct lock {
    struct thread *holder;        /*!< Thread holding lock (for debugging). */
    struct semaphore semaphore;   /*!< Binary semaphore controlling access. */
    struct list_elem elem;        /*!< List element. */
    struct list blocked_threads;  /*!< Threads this lock is blocking */
};

void lock_init(struct lock *);
void lock_acquire(struct lock *);
bool lock_try_acquire(struct lock *);
void lock_release(struct lock *);
bool lock_held_by_current_thread(const struct lock *);
int calc_lock_priority(struct lock *lock);

/*! Condition variable. */
struct condition {
    struct list waiters;        /*!< List of waiting threads. */
};

void cond_init(struct condition *);
void cond_wait(struct condition *, struct lock *);
void cond_signal(struct condition *, struct lock *);
void cond_broadcast(struct condition *, struct lock *);

enum rw_state {
  READ,
  WRITE,
  NONE,
  READER_WAIT,
  WRITER_WAIT,
};

struct rw_lock {
  struct lock read_lock;        /*!< Lock to change and access variables. */
  int num_readers;              /*!< Number of blocking readers. */
  struct condition read_cond;   /*!< Condition to signal all readers. */
  struct condition write_cond;  /*!< Condition to signal writers. */
  enum rw_state state;          /*!< State lock is currently in. */
};

void rw_lock_init(struct rw_lock *rw);
void begin_read(struct rw_lock *rw);
void end_read(struct rw_lock *rw);
void begin_write(struct rw_lock *rw);
void end_write(struct rw_lock *rw);

/*! Optimization barrier.

   The compiler will not reorder operations across an
   optimization barrier.  See "Optimization Barriers" in the
   reference guide for more information.*/
#define barrier() asm volatile ("" : : : "memory")

#endif /* threads/synch.h */

