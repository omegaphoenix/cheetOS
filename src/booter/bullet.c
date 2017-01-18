#include <stdio.h>
#include <stdlib.h>

#include "bullet.h"

Bullet *Bullet_new(int x_pos,
                   int y_pos,
                   Direction direction,
                   ShooterType source,
                   int speed) {
    Bullet *new_bullet = malloc(sizeof(Bullet));

    if (new_bullet) {
        new_bullet->next_bullet = NULL;
        new_bullet->prev_bullet = NULL;

        new_bullet->x_pos = x_pos;
        new_bullet->y_pos = y_pos;

        new_bullet->direction = direction;
        new_bullet->source = source;

        new_bullet->speed = speed;
        return new_bullet;
    }
    else {
        fprintf(stderr, "Error: Failed to malloc Bullet.\n");
        return NULL;
    }
}

void Bullet_free(Bullet *bullet) {
    free(bullet);
}

/* TODO: Not sure how I'll handle this yet. */
void bullet_move(Bullet *moving_bullet) {

}



BulletLinkedList *BulletLinkedList_new() {
    BulletLinkedList *bullet_LL = malloc(sizeof(BulletLinkedList));

    if (bullet_LL) {
      bullet_LL->first_bullet = NULL;
      bullet_LL->last_bullet = NULL;

      bullet_LL->num_bullets = 0;
      return bullet_LL;
    }
    else {
      fprintf(stderr, "Error: Failed to malloc BulletLinkedList.\n");
      return NULL;
    }
}

void BulletLinkedList_free(BulletLinkedList *bullet_LL) {
  if (bullet_LL) {
    Bullet *curr_bullet = bullet_LL->first_bullet;
    Bullet *temp_bullet;

    /* Keep freeing until all nodes are freed */
    while (curr_bullet) {
        temp_bullet = curr_bullet->next_bullet;
        Bullet_free(curr_bullet);
        curr_bullet = temp_bullet;
    }

    free(bullet_LL);
  }
}

void bullet_linked_list_append(BulletLinkedList *bullet_LL,
                               Bullet *bullet) {
    Bullet *last_bullet = bullet_LL->last_bullet;
    if (last_bullet) {
      last_bullet->next_bullet = bullet;
      bullet->prev_bullet = last_bullet;

      bullet_LL->last_bullet = bullet;
    }
    else {
      bullet_LL->first_bullet = bullet;
      bullet_LL->last_bullet = bullet;
    }

    bullet_LL->num_bullets++;
}

void bullet_linked_list_remove(BulletLinkedList *bullet_LL,
                               Bullet *bullet) {
    /* Edge case. If this is the only node inside the linked list */
    if (bullet_LL->num_bullets == 0) {
        fprintf(stderr, "Error: Cannot remove a node from an empty list.\n");
        return;
    }
    else {
        Bullet *prev_bullet = bullet->prev_bullet;
        Bullet *next_bullet = bullet->next_bullet;

        Bullet_free(bullet);

        /* Include NULL checks for these */
        if (prev_bullet) {
          prev_bullet->next_bullet = next_bullet;
        }

        if (next_bullet) {
          next_bullet->prev_bullet = prev_bullet;
        }

        bullet_LL->num_bullets--;
    }
}