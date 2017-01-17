#ifndef SHOOTER_H_
#define SHOOTER_H_

#include <stdbool.h>

#include "bullet.h"

/*
 * This struct will contain all elements necessary for
 * a shooting object.
 */
typedef struct _Shooter;
typedef struct _Shooter {
    /* Top-Left corner position of alien. */
    int x_pos;
    int y_pos;

    /* Dimension of the alien. The hitbox size, if you will */
    int x_dim;
    int y_dim;

    /* Shooter speed while moving on board. Will be an added extra. */
    /* TODO: Implement direction if necessary */
    int movement_speed;

    /* Shooter stamina */
    int health;

    /* Shooting frequency */
    int shoot_frequency;

    /* This will be in a Shooter Linked List */
    Shooter *next_shooter;
    Shooter *prev_shooter;
} Shooter;

/* Dynamic constructor for shooting */
Shooter *Shooter_new(int x_pos,
                     int y_pos,
                     int x_dim,
                     int y_dim,
                     int movement_speed,
                     int health,
                     int shoot_frequency);

/* Dynamic destructor */
void Shooter_free(Shooter *shooter);

/* Moving implementation. Lower prio for now. */
void shooter_move(Shooter *moving_shooter);

/* Shooting function for the alien. Returns a bullet */
Bullet *shooter_shoot(Shooter *shooter);

/* Function that checks if a bullet and a shooter collide */
bool shooter_check_impact(Shooter *shooter, Bullet *bullet);



/* 
 * Linked list of shooters to keep track of them while not setting
 * a maximum on the number of shooters. Doubly linked for easier manipulation.
 */
typedef struct _ShooterLinkedList {
    Shooter *first_shooter;
    Shooter *last_shooter;

    /* Size of linked list */
    int num_shooters;
} ShooterLinkedList;

/* Dynamic constructor. Returns an empty Shooter linked list */
ShooterLinkedList *ShooterLinkedList_new();

/* Dynamic destructor */
void ShooterLinkedList_free(ShooterLinkedList *shooter_LL);

/* Appends to end of linked list */
void shooter_linked_list_append(ShooterLinkedList *shooter_LL,
                                Shooter *shooter);

/*
 * Removes a shooter node from the shooter linked list.
 * Assumes that shooter argument is already in the linked list
 * so that the pointers next_shooter and prev_shooter can be used
 * instead of an O(n) search for the node.
 */
void shooter_linked_list_remove(ShooterLinkedList *shooter_LL,
                                Shooter *shooter);
#endif /* SHOOTER_H_