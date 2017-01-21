#ifndef SHOOTER_H_
#define SHOOTER_H_

#include "bullet.h"
#include "gameDefinitions.h"

/*
 * This struct will contain all elements necessary for
 * a shooting object. All shooters will be 2 by 2.
 */
typedef struct _Shooter Shooter;
typedef struct _Shooter {
    /* Top-Left corner position of alien. */
    int x_pos;
    int y_pos;

    /* Denotes if you're a player or an alien */
    ShooterType shooter_type;

    /* Actual image of the shooter. Will be a 2 by 2 box. */
    char portrait[SHOOTER_WIDTH * SHOOTER_HEIGHT];

    /* Shooter speed while moving on board. Will be an added extra. */
    int movement_speed;

    /* Shooter stamina */
    int health;

    /* Shooting frequency */
    int shoot_frequency;

    /* 1 if visible, 0 if not visible. Dead ones should not be visible */
    int visible;
} Shooter;

/*
 * For the sake of ___stack_chk_fail, we will not be returning any
 * structs and instead, we will pass them in by reference
 */
/* shooter assignment */
void new_shooter(Shooter *shooter,
                 int x_pos,
                 int y_pos,
                 int movement_speed,
                 ShooterType shooter_type,
                 int health,
                 int shoot_frequency);

/* Moving implementation. Lower prio for now. */
void shooter_move(Shooter *moving_shooter, int left);

/* Shooting function for the alien. Returns a bullet */
void shooter_shoot(Shooter *shooter, Bullet *bullet);

/*
 * Function that checks if a bullet and a shooter collide.
 * Returns 0 if false, 1 if true.
 */
void shooter_handle_impact(Shooter *shooter, Bullet *bullet);

/* Checks if shooter is still alive */
void shooter_check_health(Shooter *shooter);

#endif /* SHOOTER_H_ */
