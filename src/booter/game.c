#include "bullet.h"
#include "game.h"
#include "shooter.h"

Game Game_new(int x_dim, int y_dim, int difficulty_level) {
    Game new_game;
    int idx;
    /* Initialize the player */
    new_game.player = Shooter_new(x_dim / 2, y_dim / 2, 1, PLAYER, 100, 0);

    for (idx = 0; idx < 5; idx++) {
      new_game.aliens[idx] = Shooter_new(idx * 5 + 10, idx * 5 + 10, 1, ALIEN, 30, idx + 1);
    }

    new_game.x_dim = x_dim;
    new_game.y_dim = y_dim;
    new_game.difficulty_level = difficulty_level;
    new_game.num_bullets = 0;

    return new_game;
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

    /* For testing */
    *((int*)0xB8000) = 0x07690748;

    /* Loop forever, so that we don't fall back into the bootloader code. */
    while (1) {}
}

