#include <stdio.h>
#include <stdlib.h>

#include "shooter.h"

/*
 * All function definitions are in the header file.
 */

Shooter Shooter_new(int x_pos,
                    int y_pos,
                    int movement_speed,
                    ShooterType shooter_type,
                    int health,
                    int shoot_frequency) {
    int idx;
    Shooter new_shooter;

    new_shooter.x_pos = x_pos;
    new_shooter.y_pos = y_pos;

    new_shooter.shooter_type = shooter_type;

    if (shooter_type == ALIEN) {
      for (idx = 0; idx < 4; idx++) {
        new_shooter.portrait[idx] = 'v';
      }
    }
    else {
      for (idx = 0; idx < 4; idx++) {
        new_shooter.portrait[idx] = '^';
      }
    }

    new_shooter.movement_speed = movement_speed;
    new_shooter.health = health;
    new_shooter.shoot_frequency = shoot_frequency;
    new_shooter.visible = 1; /* All start off visible */

    return new_shooter;
}

/* TODO: Will probably similar to frequency of shooting */
void shooter_move(Shooter *moving_shooter) {

}

Bullet shooter_shoot(Shooter *shooter) {
    /* For now, spawn bullets in the top left */
    Direction bullet_direction;
    Bullet new_bullet;

    if (shooter->shooter_type == PLAYER) {
        bullet_direction = UP;
    }
    else {
        bullet_direction = DOWN;
    }

    /* For now, all bullets fly at same speed */
    /* TODO: Customize bullet speed if we have time */
    new_bullet = Bullet_new(shooter->x_pos,
                            shooter->y_pos,
                            bullet_direction,
                            shooter->shooter_type,
                            5);

    return new_bullet;
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
        return 1;
    }
    return 0;
}
