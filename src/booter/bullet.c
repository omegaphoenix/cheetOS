#include "bullet.h"

void new_bullet(Bullet *bullet,
                int x_pos,
                int y_pos,
                Direction direction,
                ShooterType source,
                int speed) {
    bullet->x_pos = x_pos;
    if (source == PLAYER) {
        bullet->y_pos = y_pos - 1;
    }

    /* Position is top left, so you have to add two to aliens */
    else {
        bullet->y_pos = y_pos + 2;
    }
    bullet->direction = direction;
    bullet->source = source;

    bullet->speed = speed;
    bullet->visible = 1;
}

/* TODO: Not sure how I'll handle this yet. */
void bullet_move(Bullet *moving_bullet) {
    if (moving_bullet->direction == UP) {
        moving_bullet->y_pos--;
    }
    else {
        moving_bullet->y_pos++;
    }
}
