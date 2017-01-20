#include "game.h"

#include "bullet.h"
#include "gameDefinitions.h"
#include "interrupts.h"
#include "keyboard.h"
#include "shooter.h"
#include "timer.h"
#include "video.h"

/* Starting random seed */
void rand() {
    game.seed = (RANDOM_A * game.seed + RANDOM_C) % RANDOM_M;
}

void next_level() {
    int idx;
    game.score = game.score + game.difficulty_level * 50;
    game.difficulty_level = game.difficulty_level + 1;

    /*
     * These values were set arbitrarily;
     * they didn't seem to overwhelm the player
     */
    int level_bullet_speed = (7 - game.difficulty_level) * 2;

    /* To smooth shooting frequencies, generate two random numbers */
    int random_shoot_freq;
    int smooth_random_freq;

    game.player.health++;

    for (idx = 0; idx < NUM_ALIENS; idx++) {
        rand();
        random_shoot_freq = (game.seed % 10) + 1;

        rand();
        smooth_random_freq = (game.seed % 10) + 1;
        new_shooter(&game.aliens[idx],
                    idx * 7 + 5,
                    0,
                    1,
                    ALIEN,
                    3,
                    (random_shoot_freq + level_bullet_speed) *
                    (smooth_random_freq + level_bullet_speed));
    }

    /* Redraw aliens */
    for (idx = 0; idx < NUM_ALIENS; idx++) {
        draw_shooter(game.aliens[idx]);
    }

    /* Clear old bullets */
    for (idx = 0; idx < NUM_BULLETS; idx++) {
        clear_bullet(game.bullets[idx]);
        game.bullets[idx].visible = 0;
    }
}


void new_game(int x_dim, int y_dim, int difficulty_level) {
    int idx;
    /* Set initial game seed to any number */
    game.seed = 666;
    game.difficulty_level = difficulty_level;

    /*
     * These values were set arbitrarily;
     * they didn't seem to overwhelm the player
     */
    int level_bullet_speed = (7 - game.difficulty_level) * 2;

    /* To smooth shooting frequencies, generate two random numbers */
    int random_shoot_freq;
    int smooth_random_freq;

    new_shooter(&game.player, x_dim / 2, y_dim - 3, 1, PLAYER, 10, 0);

    for (idx = 0; idx < NUM_ALIENS; idx++) {
        rand();
        random_shoot_freq = (game.seed % 10) + 1;

        rand();
        smooth_random_freq = (game.seed % 10) + 1;
        new_shooter(&game.aliens[idx],
                    idx * 7 + 5,
                    0,
                    1,
                    ALIEN,
                    3,
                    (random_shoot_freq + level_bullet_speed) *
                    (smooth_random_freq + level_bullet_speed));
    }

    /* For now, all we need is for the bullets to not be visible. */
    for (idx = 0; idx < NUM_BULLETS; idx++) {
        game.bullets[idx].visible = 0;
    }

    game.x_dim = x_dim;
    game.y_dim = y_dim;
    game.score = 0;
}

/*
 * Handles bullet movement. Will deal with shooting frequency later once I see how
 * timer works
 */
void update_game(int timer_count) {
    int alien_idx;
    int bullet_idx;

    /* Iterate through bullets to update their movement */
    for (bullet_idx = 0; bullet_idx < NUM_BULLETS; bullet_idx++) {
        if (game.bullets[bullet_idx].visible == 1) {

            /* First, update movements. then, check collisions */
            clear_bullet(game.bullets[bullet_idx]);
            bullet_move(&game.bullets[bullet_idx], timer_count);

            /* Check potential impacts */
            shooter_handle_impact(&game.player, &game.bullets[bullet_idx]);
            for (alien_idx = 0; alien_idx < NUM_ALIENS; alien_idx++) {
                if (game.aliens[alien_idx].visible) {
                    shooter_handle_impact(&game.aliens[alien_idx],
                                          &game.bullets[bullet_idx]);
                    if (game.aliens[alien_idx].visible == 0) {
                        game.score += 5 * game.difficulty_level;
                    }
                }
            }

            /* If bullet goes offscreen */
            if (game.bullets[bullet_idx].y_pos > game.y_dim ||
                game.bullets[bullet_idx].y_pos < 0) {
                game.bullets[bullet_idx].visible = 0;
            }

            /* If not impacted, draw it again */
            if (game.bullets[bullet_idx].visible == 1) {
                draw_bullet(game.bullets[bullet_idx]);
            }
        }
    }

    if (timer_count > 0 && !is_empty_queue()) {
        unsigned char key_code = dequeue();
        switch (key_code) {
            case LEFT_KEY:
                shooter_move(&game.player, 1);
                break;
            case RIGHT_KEY:
                shooter_move(&game.player, 0);
                break;
            case UP_KEY:
            case SPACE_KEY:
                create_or_replace_bullet(&game, &game.player);
                break;
            default:
                break;
        }
    }

    /* Iterate through aliens and make some of them shoot */
    for (alien_idx = 0; alien_idx < NUM_ALIENS; alien_idx++) {
        if (game.aliens[alien_idx].visible == 1) {
            int shoot_frequency = game.aliens[alien_idx].shoot_frequency;
            if ((timer_count % shoot_frequency == 0 ||
                 timer_count % shoot_frequency == 5) &&
                timer_count > shoot_frequency / 2) {
                create_or_replace_bullet(&game, &game.aliens[alien_idx]);
            }
        }
    }

    update_score(game.score);
    display();

    if (is_game_finished(&game)) {
        if (game.player.visible) {
            next_level();
        }
    }
}

/* 0 is false. 1 is game over or next level. */
int is_game_finished(Game *game) {
    int idx;

    /* If player is dead */
    if (!game->player.visible) {
        return 1;
    }

    for (idx = 0; idx < NUM_ALIENS; idx++) {
        if (game->aliens[idx].visible) {
          return 0;
        }
    }
    return 1;
}

void create_or_replace_bullet(Game *game, Shooter *shooter) {
    int idx;

    for (idx = 0; idx < NUM_BULLETS; idx++) {
        if (game->bullets[idx].visible == 0) {
            shooter_shoot(shooter, &game->bullets[idx]);
            return;
        }
    }
    return ;
}

/* This is the entry-point for the game! */
void c_start(void) {
    /* TODO:  You will need to initialize various subsystems here.  This
     *        would include the interrupt handling mechanism, and the various
     *        systems that use interrupts.  Once this is done, you can call
     *        enable_interrupts() to start interrupt handling, and go on to
     *        do whatever else you decide to do!
     */
    int idx;

    new_game(80, 25, 1);

    init_video();

    /* initial drawings - move these to init_video() */
    draw_shooter(game.player);
    for (idx = 0; idx < NUM_ALIENS; idx++) {
        draw_shooter(game.aliens[idx]);
    }
    create_or_replace_bullet(&game, &game.player);
    display();

    update_game(0);
    init_interrupts();
    init_keyboard();
    init_timer();

    enable_interrupts();

    /* Loop forever, so that we don't fall back into the bootloader code. */
    while (1) {
    }
}

