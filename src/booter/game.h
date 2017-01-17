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
    Shooter *player;

    /* Alien shooters */
    ShooterLinkedList *aliens;

    /* Bullets */
    BulletLinkedList *bullets;

    /* Difficulty Level */
    int difficulty_level;
} Game;

/* Instantiate Game. Player will start at a preset speed. */
Game *Game_new(int x_dim, int y_dim, int difficulty_level);

/* Dynamic destructor */
void Game_free(Game *game);

/* Updates the game. This will become more fleshed out over time */
void Game_update(Game *game);

#endif /* GAME_H_ */
