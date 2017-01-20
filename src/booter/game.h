#ifndef GAME_H_
#define GAME_H_

#include "bullet.h"
#include "shooter.h"

#define NUM_ALIENS 10
#define NUM_BULLETS 200
/*
 * These parameters are for a pseudo random number generator
 * https://en.wikipedia.org/wiki/Linear_congruential_generator
 */
#define RANDOM_A 8121
#define RANDOM_C 28411
#define RANDOM_M 134456
/* This struct will contain all the game information necessary */
typedef struct _Game {
    /* Game board dimension */
    int x_dim;
    int y_dim;

    /* Player shooter */
    Shooter player;

    /* Alien shooters */
    Shooter aliens[NUM_ALIENS];

    /* Bullets */
    Bullet bullets[NUM_BULLETS];

    /* Difficulty Level */
    int difficulty_level;

    /* Random seed */
    int seed;

    int score;
} Game;

Game game;

/* Instantiate Game. Player will start at a preset speed. */
void new_game(int x_dim, int y_dim, int difficulty_level);

/* Updates the game. This will become more fleshed out over time */
void update_game();

/*
 * Checks if the game is finished. The game finishes if:
 * 1) Player dies OR
 * 2) All aliens are dead
 */
int is_game_finished(Game *game);

/*
 * If the maximum of 10 bullets is reached, this will just replace
 * Any bullet that is now invisible
 */
void create_or_replace_bullet(Game *game, Shooter *shooter);

/*
 * Implementation of random0 using Linear Congruential Generator
 * https://en.wikipedia.org/wiki/Linear_congruential_generator
 */
void rand();
#endif /* GAME_H_ */
