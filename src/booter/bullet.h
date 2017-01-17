#ifndef BULLET_H_
#define BULLET_H_

#include <stdbool.h>

#include "gameDefinitions.h"

typedef struct _Bullet Bullet;
typedef struct _Bullet {
    /* Position for bullet */
    int x_pos;
    int y_pos;

    /* Denotes what direction the bullet moves */
    Direction direction;

    /* Denotes if an alien shot it or a player. Friendly fire off */
    Sender source;

    /* Speed of the bullet */
    int speed;

    /* Bullets will also be stored in a linked list */
    Bullet *next_bullet;
    Bullet *prev_bullet;
} Bullet;

/* Dynamic constructor for bullets */
Bullet *Bullet_new(int x_pos,
                   int y_pos,
                   Direction direction,
                   Sender source,
                   int speed);

/* Dynamic destructor for bullets */
void Bullet_free(Bullet *bullet);

/* Move update for the bullet */
void bullet_move(Bullet *moving_bullet);


/*
 * Linked list of bullets to keep track of them without setting a maximum
 * limit on bullets. Doubly linked for easier manipulation.
 */
typedef struct _BulletLinkedList {
    Bullet *first_bullet;
    Bullet *last_bullet;

    /* Size of linked list */
    int num_bullets;
} BulletLinkedList;

/* Dynamic constructor. Returns empty Bullet linked list */
BulletLinkedList *BulletLinkedList_new();

/* Dynamic destructor */
void BulletLinkedList_free(BulletLinkedList *bullet_LL);

/* Appends to end of linked list */
void bullet_linked_list_append(BulletLinkedList *bullet_LL,
                               Bullet *bullet);

/*
 * Removes a bullet node from the bullet linked list.
 * Assumes that bullet argument is already in the linked list
 * so that the pointers next_bullet and prev_bullet can be used
 * instead of an O(n) search for the node.
 */
void bullet_linked_list_remove(BulletLinkedList *bullet_LL,
                               Bullet *bullet);
#endif /* BULLET_H_ */