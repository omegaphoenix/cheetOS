#include <stdio.h>
#include <stdlib.h>

#include "shooter.h"

/*
 * All function definitions are in the header file.
 */

Shooter *Shooter_new(int x_pos,
                     int y_pos,
                     int x_dim,
                     int y_dim,
                     int movement_speed,
                     ShooterType shooter_type,
                     int health,
                     int shoot_frequency) {
    int idx;
    Shooter *new_shooter = malloc(sizeof(Shooter));
    char *portrait = malloc(x_dim * y_dim + 1); /* For end of string */

    if (portrait && new_shooter) {

        new_shooter->next_shooter = NULL;
        new_shooter->prev_shooter = NULL;

        new_shooter->x_pos = x_pos;
        new_shooter->y_pos = y_pos;

        new_shooter->x_dim = x_dim;
        new_shooter->y_dim = y_dim;

        new_shooter->shooter_type = shooter_type;

        /* TODO: Maybe customized portraits? For now, just two portraits. */
        if (shooter_type == ALIEN) {
          for (idx = 0; idx < x_dim * y_dim; idx++) {
            portrait[idx] = 'v';
          }
          portrait[x_dim * y_dim] = '\0';
        }
        else {
          for (idx = 0; idx < x_dim * y_dim; idx++) {
            portrait[idx] = '^';
          }
          portrait[x_dim * y_dim] = '\0';
        }
        new_shooter->portrait = portrait;

        new_shooter->movement_speed = movement_speed;
        new_shooter->health = health;
        new_shooter->shoot_frequency = shoot_frequency;

        return new_shooter;
    }
    else {
      fprintf(stderr, "Error: failed to malloc Shooter.\n");
      return NULL;
    }
}

/* Have to check if null first. */
void Shooter_free(Shooter *shooter) {
    if (shooter) {
        free(shooter->portrait);
        free(shooter);
    }
}

/* TODO: Will probably similar to frequency of shooting */
void shooter_move(Shooter *moving_shooter) {

}

Bullet *shooter_shoot(Shooter *shooter) {
    /* For now, spawn bullets in the top left */
    Direction bullet_direction;
    Bullet *new_bullet = NULL;
    if (shooter->shooter_type == ALIEN) {
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

bool shooter_check_impact(Shooter *shooter, Bullet *bullet) {
    /* Check if bullet is inside the shooter's box */
    if (bullet->x_pos < shooter->x_pos + shooter->x_dim &&
        bullet->x_pos >= shooter->x_pos &&
        bullet->y_pos < shooter->y_pos + shooter->y_dim &&
        bullet->y_pos >= shooter->y_pos) {
        return true;
    }
    return false;
}



ShooterLinkedList *ShooterLinkedList_new() {
    ShooterLinkedList *shooter_LL = malloc(sizeof(ShooterLinkedList));

    if (shooter_LL) {
      shooter_LL->first_shooter = NULL;
      shooter_LL->last_shooter = NULL;

      shooter_LL->num_shooters = 0;
      return shooter_LL;
    }
    else {
      fprintf(stderr, "Error: Failed to malloc ShooterLinkedList.\n");
      return NULL;
    }
}

void ShooterLinkedList_free(ShooterLinkedList *shooter_LL) {
    if (shooter_LL) {
        Shooter *curr_shooter = shooter_LL->first_shooter;
        Shooter *temp_shooter;

        /* Keep freeing until all nodes are freed */
        while (curr_shooter) {
          temp_shooter = curr_shooter->next_shooter;
          Shooter_free(curr_shooter);
          curr_shooter = temp_shooter;
        }

        free(shooter_LL);
    }
}

void shooter_linked_list_append(ShooterLinkedList *shooter_LL,
                                Shooter *shooter) {
    Shooter *last_shooter = shooter_LL->last_shooter;
    if (last_shooter) {
        last_shooter->next_shooter = shooter;
        shooter->prev_shooter = last_shooter;

        shooter_LL->last_shooter = shooter;
    }
    else {
        shooter_LL->first_shooter = shooter;
        shooter_LL->last_shooter = shooter;
    }

    shooter_LL->num_shooters++;
}

/*
 * Assumes that shooter is a part of shooter_LL, so it has 
 * a prev and a next pointer.
 */
void shooter_linked_list_remove(ShooterLinkedList *shooter_LL,
                                Shooter *shooter) {
    /* Edge case. If this is the only node inside the linked list */
    if (shooter_LL->num_shooters == 0) {
        fprintf(stderr, "Error: Cannot remove a node from an empty list.\n");
        return;
    }
    else {
        Shooter *prev_shooter = shooter->prev_shooter;
        Shooter *next_shooter = shooter->next_shooter;

        Shooter_free(shooter);

        /* Include NULL checks for these */
        if (prev_shooter) {
          prev_shooter->next_shooter = next_shooter;
        }

        if (next_shooter) {
          next_shooter->prev_shooter = prev_shooter;
        }

        shooter_LL->num_shooters--;
    }
}
