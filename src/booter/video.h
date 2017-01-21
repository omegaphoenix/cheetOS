#ifndef VIDEO_H
#define VIDEO_H

#include "shooter.h"

/* Available colors from the 16-color palette used for EGA and VGA, and
 * also for text-mode VGA output.
 */
#define BLACK          0
#define BLUE           1
#define GREEN          2
#define CYAN           3
#define RED            4
#define MAGENTA        5
#define BROWN          6
#define LIGHT_GRAY     7
#define DARK_GRAY      8
#define LIGHT_BLUE     9
#define LIGHT_GREEN   10
#define LIGHT_CYAN    11
#define LIGHT_RED     12
#define LIGHT_MAGENTA 13
#define YELLOW        14
#define WHITE         15

#define MAX_SCORE     999999
#define MAX_DIGITS    6

#define GRID_HEIGHT 25
#define GRID_WIDTH 80

void init_video(void);

/* Clears the grid but does not change the video buffer */
void clear();

/* Clears bullet from grid but not display */
void clear_bullet(Bullet bullet);

/* Clears shooter from grid but not display. */
void clear_shooter(Shooter actor);

/* Writes the contents of the grid to the video buffer */
void display();

/* Draws an actor by adding its icon and color to the grid */
void draw_shooter(Shooter actor);

/* Draws a bullet by adding its icon and color to grid */
void draw_bullet(Bullet bullet);

/* Sets background color for entire display */
void set_bg_color(char color);

/* Set pixel character and color */
void set_pix(int x, int y, char color, char character);

/* Set grid pixel character and color to be updated on display() */
void set_grid_pix(int x, int y, char color, char character);

/* Return color byte using bg and given color */
char get_color(char color, char bg_col);

/* Return color stored at pixel in grid */
char get_grid_color(int x, int y);

/* Return character stored at pixel in grid */
char get_grid_char(int x, int y);

/* Print insulting comment */
void game_over();

/* Initialize score on screen */
void init_score();

/* Update on-screen score */
void update_score(int score);
#endif /* VIDEO_H */
