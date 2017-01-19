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
#define VIDEO_BUFFER ((char *) 0xB8000)
#define GRID_HEIGHT 25
#define GRID_WIDTH 80
#define COLOR_OFFSET 1


/* TODO:  You can create static variables here to hold video display state,
 *        such as the current foreground and background color, a cursor
 *        position, or any other details you might want to keep track of!
 */

/* TODO: holds character and color. Need 2-byte type. Initialize to 0s? */
static int grid[GRID_HEIGHT][GRID_WIDTH];
static int bg_color;

int get_pix_offset(int x, int y) {
    int loc = 2 * ((GRID_WIDTH * y) + x);
}

char get_color(char color) {
    return (bg_color << 4) + color;
}

void set_pix(int x, int y, char color, char character) {
    int loc = get_pix_offset(x, y);
    char *video = VIDEO_BUFFER + loc;
    *video = character;
    *(video + COLOR_OFFSET) = get_color(color);
}

void clear() {
    /* TODO: reset grid to default values */
    return 0;
}

void display() {
    /* TODO: loop through grid and add char, fg color, bg color to buffer */
    int default_bg_color = BLACK;
    void *video = VIDEO_BUFFER;
    set_bg_color(default_bg_color);
}

void draw(int actor, int color) {
    /* TODO: store char and fg color in the grids */
};

void set_bg_color(int color) {
    bg_color = color;
    int i, x, y;
    for (i = 0; i < GRID_HEIGHT * GRID_WIDTH; i++) {
        x = i % GRID_WIDTH;
        y = i / GRID_WIDTH;
        set_pix(x, y, bg_color, ' ');
    }
}

void init_video(void) {
    /* TODO:  Do any video display initialization you might want to do, such
     *        as clearing the screen, initializing static variable state, etc.
     */
    bg_color = YELLOW;
    set_bg_color(bg_color);
    set_pix(1, 1, MAGENTA, 'I');
    set_pix(2, 1, MAGENTA, '\'');
    set_pix(3, 1, MAGENTA, 'M');
    set_pix(4, 1, MAGENTA, ' ');
    set_pix(5, 1, MAGENTA, 'A');
    set_pix(6, 1, MAGENTA, ' ');
    set_pix(7, 1, MAGENTA, 'D');
    set_pix(8, 1, MAGENTA, 'W');
    set_pix(9, 1, MAGENTA, 'E');
    set_pix(10, 1, MAGENTA, 'E');
    set_pix(11, 1, MAGENTA, 'B');
}

