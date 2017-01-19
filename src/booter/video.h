#ifndef VIDEO_H
#define VIDEO_H


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


void init_video(void);

/* Clears the grid but does not change the video buffer */
void clear();

/* Writes the contents of the grid to the video buffer */
void display();

/* Draws an actor by adding its icon and color to the grid */
void draw(int actor, int color);

/* Sets background color for entire display */
void set_bg_color(int color);

/* Set pixel character and color */
void set_pix(int x, int y, char color, char character);

/* Return color byte using bg and given color */
char get_color(char color);
#endif /* VIDEO_H */
