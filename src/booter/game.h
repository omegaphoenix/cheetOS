#ifndef GAME_H_
#define GAME_H_

#include "bullet.h"
#include "shooter.h"

/* This struct will contain all the game information necessary */
typedef struct _Game {
    /* Game board dimension */
    int x_dim;
    int y_dim;

    /* Player shooter */
    Shooter player;

    /* Alien shooters */
    Shooter aliens[5];

    /* Bullets */
    Bullet bullets[10];

    /* Current number of bullets. Only needed until array gets full once. */
    int num_bullets;

    /* Difficulty Level */
    int difficulty_level;
} Game;

Game game;

/* Instantiate Game. Player will start at a preset speed. */
void new_game(int x_dim, int y_dim, int difficulty_level);

/* Updates the game. This will become more fleshed out over time */
void update_game(Game *game);

#endif /* GAME_H_ */
