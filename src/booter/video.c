#include "video.h"

/* This is the address of the VGA text-mode video buffer.  Note that this
 * buffer actually holds 8 pages of text, but only the first page (page 0)
 * will be displayed.
 *
 * Individual characters in text-mode VGA are represented as two adjacent
 * bytes:
 *     Byte 0 = the character value
 *     Byte 1 = the color of the character:  the high nibble is the background
 *              color, and the low nibble is the foreground color
 *
 * See http://wiki.osdev.org/Printing_to_Screen for more details.
 *
 * Also, if you decide to use a graphical video mode, the active video buffer
 * may reside at another address, and the data will definitely be in another
 * format.  It's a complicated topic.  If you are really intent on learning
 * more about this topic, go to http://wiki.osdev.org/Main_Page and look at
 * the VGA links in the "Video" section.
 */
#define VIDEO_BUFFER ((void *) 0xB8000)
#define GRID_HEIGHT 25
#define GRID_WIDTH 80


/* TODO:  You can create static variables here to hold video display state,
 *        such as the current foreground and background color, a cursor
 *        position, or any other details you might want to keep track of!
 */

/* TODO: holds character and color. Need 2-byte type. Initialize to 0s? */
static int grid[GRID_HEIGHT][GRID_WIDTH];
static int bg_color;



int clear() {
    /* TODO: reset grid to default values */
    return 0;
}

int display() {
    /* TODO: loop through grid and add char, fg color, bg color to buffer */
    void *video = VIDEO_BUFFER;
    return 0;
}

int draw(int actor, int color) {
    /* TODO: store char and fg color in the grids */
    return 0;
};

int set_bg_color(int color) {
    bg_color = color;
}

void init_video(void) {
    /* TODO:  Do any video display initialization you might want to do, such
     *        as clearing the screen, initializing static variable state, etc.
     */
}

