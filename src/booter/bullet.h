#ifndef BULLET_H_
#define BULLET_H_

#include "gameDefinitions.h"

typedef struct _Bullet Bullet;
typedef struct _Bullet {
    /* Position for bullet */
    int x_pos;
    int y_pos;

    /* Denotes what direction the bullet moves */
    Direction direction;

    /* Denotes if an alien shot it or a player. Friendly fire off */
    ShooterType source;

    /* Speed of the bullet */
    int speed;

    /* If impacted, 0. If it hasn't impacted, 1. */
    int visible;
} Bullet;

/* Bullet assignment */
void new_bullet(Bullet *bullet,
                int x_pos,
                int y_pos,
                Direction direction,
                ShooterType source,
                int speed);

/* Move update for the bullet */
void bullet_move(Bullet *moving_bullet, int timer_count);

#endif /* BULLET_H_ */
