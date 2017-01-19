#include <stdio.h>
#include <stdlib.h>

#include "shooter.h"
#include "video.h"
/*
 * All function definitions are in the header file.
 */

void new_shooter(Shooter *new_shooter,
                    int x_pos,
                    int y_pos,
                    int movement_speed,
                    ShooterType shooter_type,
                    int health,
                    int shoot_frequency) {
    int idx;

    new_shooter->x_pos = x_pos;
    new_shooter->y_pos = y_pos;

    new_shooter->shooter_type = shooter_type;

    if (shooter_type == ALIEN) {
      for (idx = 0; idx < 4; idx++) {
        new_shooter->portrait[idx] = 'v';
      }
    }
    else {
      for (idx = 0; idx < 4; idx++) {
        new_shooter->portrait[idx] = '^';
      }
    }

    new_shooter->movement_speed = movement_speed;
    new_shooter->health = health;
    new_shooter->shoot_frequency = shoot_frequency;
    new_shooter->visible = 1; /* All start off visible */
}

/* TODO: Will probably similar to frequency of shooting */
void shooter_move(Shooter *moving_shooter, int x_mov, int y_mov) {
    clear_shooter(*moving_shooter);

    /* TODO: Do some moving poop */
    moving_shooter->x_pos += x_mov;
    moving_shooter->y_pos += y_mov;

    draw_shooter(*moving_shooter);
}

void shooter_shoot(Shooter *shooter, Bullet *bullet) {
    /* For now, spawn bullets in the top left */
    Direction bullet_direction;

    if (shooter->shooter_type == PLAYER) {
        bullet_direction = UP;
    }
    else {
        bullet_direction = DOWN;
    }

    /* For now, all bullets fly at same speed */
    /* TODO: Customize bullet speed if we have time */

    new_bullet(bullet,
               shooter->x_pos,
               shooter->y_pos,
               bullet_direction,
               shooter->shooter_type,
               5);

}

int shooter_check_impact(Shooter *shooter, Bullet *bullet) {
    /* Check if bullet is inside the shooter's box */
    if (bullet->x_pos < shooter->x_pos + 2 &&
        bullet->x_pos >= shooter->x_pos &&
        bullet->y_pos < shooter->y_pos + 2 &&
        bullet->y_pos >= shooter->y_pos &&
        bullet->source != shooter->shooter_type) {

        /* Decrease health, and remove bullet */
        bullet->visible = 0;
        shooter->health -= 15;
        shooter_check_health(shooter);
        return 1;
    }
    return 0;
}

void shooter_check_health(Shooter *shooter) {
    if (shooter->health <= 0) {
        shooter->visible = 0;
    }
}