#include "keyboard.h"

#include "handlers.h"
#include "interrupts.h"
#include "ports.h"

/* This is the IO port of the PS/2 controller, where the keyboard's scan
 * codes are made available.  Scan codes can be read as follows:
 *
 *     unsigned char scan_code = inb(KEYBOARD_PORT);
 *
 * Most keys generate a scan-code when they are pressed, and a second scan-
 * code when the same key is released.  For such keys, the only difference
 * between the "pressed" and "released" scan-codes is that the top bit is
 * cleared in the "pressed" scan-code, and it is set in the "released" scan-
 * code.
 *
 * A few keys generate two scan-codes when they are pressed, and then two
 * more scan-codes when they are released.  For example, the arrow keys (the
 * ones that aren't part of the numeric keypad) will usually generate two
 * scan-codes for press or release.  In these cases, the keyboard controller
 * fires two interrupts, so you don't have to do anything special - the
 * interrupt handler will receive each byte in a separate invocation of the
 * handler.
 *
 * See http://wiki.osdev.org/PS/2_Keyboard for details.
 */
#define KEYBOARD_PORT 0x60

void init_queue() {
    /* True if interrupts were previously enabled */
    int interrupts_on = are_interrupts_enabled();
    if (interrupts_on) {
        disable_interrupts();
    }
    key_queue.capacity = QUEUE_LEN;
    key_queue.front = -1;
    key_queue.rear = -1;
    if (interrupts_on) {
        enable_interrupts();
    }
}

int is_empty_queue() {
    return key_queue.front == -1;
}

int is_full_queue() {
    int num_elements = (key_queue.rear + 1) % key_queue.capacity;
    return num_elements == key_queue.rear;
}

int queue_size() {
    int diff = key_queue.rear - key_queue.front;
    int size_multiple = key_queue.capacity - diff;
    int size = size_multiple % key_queue.capacity;
    return size;
}


void enqueue(unsigned char scan_code) {
    int interrupts_on = are_interrupts_enabled();
    if (interrupts_on) {
        disable_interrupts();
    }
    if (!is_full_queue(key_queue)) {
        key_queue.rear = (key_queue.rear + 1) % key_queue.capacity;
        key_queue.array[key_queue.rear] = scan_code;
        if (key_queue.front == -1) {
            key_queue.front=key_queue.rear;
        }
    }
    if (interrupts_on) {
        enable_interrupts();
    }
}

unsigned char dequeue() {
    char data = 0;

    int interrupts_on = are_interrupts_enabled();
    if (interrupts_on) {
        disable_interrupts();
    }
    if (is_empty_queue(key_queue)) {
        if (interrupts_on) {
            enable_interrupts();
        }
        return 0;
    }
    else {
        data = key_queue.array[key_queue.front];
        if (key_queue.front == key_queue.rear) {
            key_queue.front = -1;
            key_queue.rear = -1;
        }
        else {
            key_queue.front = (key_queue.front + 1) % key_queue.capacity;
        }
    }

    if (interrupts_on) {
        enable_interrupts();
    }
    return data;
}


void init_keyboard(void) {
    /* Initialize key_queue */
    init_queue();

    install_interrupt_handler(KEYBOARD_INTERRUPT, keyboard_handler);
}

void update_keyboard(void) {
    unsigned char scan_code = inb(KEYBOARD_PORT);
    enqueue(scan_code);
}
