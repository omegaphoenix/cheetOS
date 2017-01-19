#include "bullet.h"

Bullet Bullet_new(int x_pos,
                   int y_pos,
                   Direction direction,
                   ShooterType source,
                   int speed) {
    Bullet new_bullet;

    new_bullet.x_pos = x_pos;
    new_bullet.y_pos = y_pos;

    new_bullet.direction = direction;
    new_bullet.source = source;

    new_bullet.speed = speed;
    new_bullet.visible = 1;

    return new_bullet;
}

/* TODO: Not sure how I'll handle this yet. */
void bullet_move(Bullet *moving_bullet) {

}
