#include <stdio.h>
#include <stdlib.h>

#include "bullet.h"
#include "game.h"
#include "shooter.h"

Game *Game_new(int x_dim, int y_dim, int difficulty_level) {
    Game *new_game = malloc(sizeof(Game));

    /* Initialize the player */
    Shooter *player =
        Shooter_new(x_dim / 2, y_dim / 2, 1, 1, 1, PLAYER, 100, 1);

    /* Initialize the alien linked list */
    ShooterLinkedList *aliens = ShooterLinkedList_new();

    /* Initialize the bullet linked list */
    BulletLinkedList *bullets = BulletLinkedList_new();

    if (new_game && player && aliens && bullets) {
        new_game->x_dim = x_dim;
        new_game->y_dim = y_dim;
        new_game->difficulty_level = difficulty_level;

        new_game->player = player;
        new_game->aliens = aliens;
        new_game->bullets = bullets;
        return new_game;
    }
    else {
      fprintf(stderr, "Error: Failed to malloc Game components.\n");
      return NULL;
    }
}

void Game_free(Game *game) {
    Shooter_free(game->player);
    ShooterLinkedList_free(game->aliens);
    BulletLinkedList_free(game->bullets);
    free(game);
}


/* TODO: Game updates */
void Game_update(Game *game) {

}



/* This is the entry-point for the game! */
void c_start(void) {
    /* TODO:  You will need to initialize various subsystems here.  This
     *        would include the interrupt handling mechanism, and the various
     *        systems that use interrupts.  Once this is done, you can call
     *        enable_interrupts() to start interrupt handling, and go on to
     *        do whatever else you decide to do!
     */

    Game *new_game = Game_new(50, 50, 1);

    /* Loop forever, so that we don't fall back into the bootloader code. */
    while (1) {}
}

