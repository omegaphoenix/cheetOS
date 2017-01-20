#ifndef KEYBOARD_H
#define KEYBOARD_H

#define LEFT_KEY 0x4b
#define RIGHT_KEY 0x4d

#define QUEUE_LEN 100

typedef struct CircQueue {
    int front, rear;
    int capacity;
    unsigned char array[QUEUE_LEN];
} CircQueue;

volatile CircQueue key_queue;

/* Initialize key_queue */
void init_queue();

/* Return true if empty */
int is_empty_queue();

/* Return true if full */
int is_full_queue();

/* Return number of keys in key_queue */
int queue_size();

/* Add key to key_queue. Does nothing if queue is full */
void enqueue(unsigned char x);

/* Remove key from key_queue and return */
unsigned char dequeue();

/* Initialize keyboard to handle */
void init_keyboard(void);

/* Update keyboard by adding key to queue */
void update_keyboard(void);

#endif /* KEYBOARD_H */

