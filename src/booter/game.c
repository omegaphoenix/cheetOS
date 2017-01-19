#include "bullet.h"
#include "game.h"
#include "shooter.h"

void init_game(int x_dim, int y_dim, int difficulty_level) {
    int idx;
    /* Initialize the player */
    // game.player = Shooter_new(x_dim / 2, y_dim / 2, 1, PLAYER, 100, 0);

    for (idx = 0; idx < 5; idx++) {
      // game.aliens[idx] = Shooter_new(idx * 5 + 10, idx * 5 + 10, 1, ALIEN, 30, idx + 1);
    }

    game.x_dim = x_dim;
    game.y_dim = y_dim;
    game.difficulty_level = difficulty_level;
    game.num_bullets = 0;
}

/* TODO: Game updates */
void update_game(Game *game) {

}



/* This is the entry-point for the game! */
void c_start(void) {
    /* TODO:  You will need to initialize various subsystems here.  This
     *        would include the interrupt handling mechanism, and the various
     *        systems that use interrupts.  Once this is done, you can call
     *        enable_interrupts() to start interrupt handling, and go on to
     *        do whatever else you decide to do!
     */

    init_game(50, 50, 1);

    /* Loop forever, so that we don't fall back into the bootloader code. */
    while (1) {}
}

