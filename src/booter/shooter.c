#include "game.h"
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
        new_shooter->portrait[0] = 219;
        new_shooter->portrait[1] = 219;
        new_shooter->portrait[2] = 254;
        new_shooter->portrait[3] = 254;
      }
    }
    else {
      for (idx = 0; idx < 4; idx++) {
        new_shooter->portrait[0] = 222;
        new_shooter->portrait[1] = 221;
        new_shooter->portrait[2] = 178;
        new_shooter->portrait[3] = 178;
      }
    }

    new_shooter->movement_speed = movement_speed;
    new_shooter->health = health;
    new_shooter->shoot_frequency = shoot_frequency;
    new_shooter->visible = 1; /* All start off visible */
}

/* TODO: Will probably similar to frequency of shooting */
void shooter_move(Shooter *moving_shooter, int left) {
    if (moving_shooter->visible) {
        clear_shooter(*moving_shooter);

        if (left) {
            moving_shooter->x_pos -= 1;
        }
        else {
            moving_shooter->x_pos += 1;
        }

        draw_shooter(*moving_shooter);
    }
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

    rand();
    new_bullet(bullet,
               shooter->x_pos + (game.seed % 2),
               shooter->y_pos,
               bullet_direction,
               shooter->shooter_type,
               2);

}

void shooter_handle_impact(Shooter *shooter, Bullet *bullet) {
    /* Check if bullet is inside the shooter's box */
    if (bullet->x_pos < shooter->x_pos + 2 &&
        bullet->x_pos >= shooter->x_pos &&
        bullet->y_pos < shooter->y_pos + 2 &&
        bullet->y_pos >= shooter->y_pos &&
        bullet->source != shooter->shooter_type) {

        /* Decrease health, and remove bullet */
        bullet->visible = 0;
        shooter->health -= 1;
        shooter_check_health(shooter);
    }
}

void shooter_check_health(Shooter *shooter) {
    /* declining health */
    if (shooter->health <= 10 && shooter->health > 5) {
        shooter->portrait[0] = 222;
        shooter->portrait[1] = 179;
        shooter->portrait[2] = 177;
        shooter->portrait[3] = 177;
        draw_shooter(*shooter);
    }
    /* low health */
    else if (shooter->health <= 5 && shooter->health > 0) {
        shooter->portrait[0] = 179;
        shooter->portrait[1] = 179;
        shooter->portrait[2] = 176;
        shooter->portrait[3] = 176;
        draw_shooter(*shooter);
    }
    /* dead */
    else if (shooter->health <= 0) {
        shooter->visible = 0;
        clear_shooter(*shooter);
    }
}
