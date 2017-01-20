#include "bullet.h"
#include "game.h"
#include "video.h"

void new_game(int x_dim, int y_dim, int difficulty_level) {
    int idx;
    new_shooter(&game.player, x_dim / 2, y_dim - 2, 1, PLAYER, 100, 0);

    for (idx = 0; idx < 5; idx++) {
        new_shooter(&game.aliens[idx], idx * 5 + 10, 0, 1, ALIEN, 15, idx + 1);
    }

    /* For now, all we need is for the bullets to not be visible. */
    for (idx = 0; idx < 10; idx++) {
        game.bullets[idx].visible = 0;
    }

    game.x_dim = x_dim;
    game.y_dim = y_dim;
    game.difficulty_level = difficulty_level;
    game.num_bullets = 0;
}

/*
 * Handles bullet movement. Will deal with shooting frequency later once I see how
 * timer works
 */
void update_game(Game *game) {
    int alien_idx;
    int bullet_idx;

    /* Iterate through bullets to update their movement */
    for (bullet_idx = 0; bullet_idx < 10; bullet_idx++) {
        if (game->bullets[bullet_idx].visible == 1) {

            /* First, update movements. then, check collisions */
            clear_bullet(game->bullets[bullet_idx]);
            bullet_move(&game->bullets[bullet_idx]);

            /* Check potential impacts */
            shooter_handle_impact(&game->player, &game->bullets[bullet_idx]);
            for (alien_idx = 0; alien_idx < 5; alien_idx++) {
                if (game->aliens[alien_idx].visible) {
                    shooter_handle_impact(&game->aliens[alien_idx],
                                          &game->bullets[bullet_idx]);
                }
            }

            /* If not impacted, draw it again */
            if (game->bullets[bullet_idx].visible == 1) {
                draw_bullet(game->bullets[bullet_idx]);
            }
        }
    }

    display();
}

/* 1 is true. 0 is false. */
int is_game_finished(Game *game) {
    int idx;

    /* If player is dead */
    if (!game->player.visible) {
        return 1;
    }

    for (idx = 0; idx < 5; idx++) {
        if (game->aliens[idx].visible) {
          return 0;
        }
    }
    return 1;
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

    init_video();
    new_game(40, 12, 1);
    draw_shooter(game.player);
    for (idx = 0; idx < 5; idx++) {
        draw_shooter(game.aliens[idx]);
    }

    shooter_shoot(&game.player, &game.bullets[0]);
    draw_bullet(game.bullets[0]);
    display();

    update_game(&game);
    /* Loop forever, so that we don't fall back into the bootloader code. */
    while (1) {
        /*
        int is_finished = is_game_finished(game);
        if (!is_finished) {
            update_game(game);
        } */
    }
}

