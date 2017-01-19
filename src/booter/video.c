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
static short grid[GRID_HEIGHT][GRID_WIDTH];
static int bg_color;

char get_grid_color(int x, int y) {
    short grid_entry = grid[y][x];
    char color = (grid_entry >> 0) & 0xff;
    return color;
}

char get_grid_char(int x, int y) {
    short grid_entry = grid[y][x];
    char character = (grid_entry >> 8) & 0xff;
    return character;
}

int get_pix_offset(int x, int y) {
    int loc = 2 * ((GRID_WIDTH * y) + x);
    return loc;
}

char get_color(char color, char bg_col) {
    return (bg_col << 4) + color;
}

void set_pix(int x, int y, char color, char character) {
    int loc = get_pix_offset(x, y);
    char *video = VIDEO_BUFFER + loc;
    *video = character;
    *(video + COLOR_OFFSET) = color;
}

void clear() {
    int default_bg_color = BLACK;
    void *video = VIDEO_BUFFER;
    set_bg_color(default_bg_color);
}

void display() {
    int i, x, y;
    char color, character;
    for (i = 0; i < GRID_HEIGHT * GRID_WIDTH; i++) {
        x = i % GRID_WIDTH;
        y = i / GRID_WIDTH;
        color = get_grid_color(x, y);
        character = get_grid_char(x, y);
        set_pix(x, y, color, character);
    }
}

void draw(int actor, char color) {
    /* TODO: store char and fg color in the grids */
};

void set_bg_color(char color) {
    bg_color = color;
}

void init_video(void) {
    /* TODO:  Do any video display initialization you might want to do, such
     *        as clearing the screen, initializing static variable state, etc.
     */
    set_bg_color(YELLOW);
    char color = get_color(MAGENTA, bg_color);
    set_pix(1, 1, color, 'I');
    set_pix(2, 1, color, '\'');
    set_pix(3, 1, color, 'M');
    set_pix(4, 1, color, ' ');
    set_pix(5, 1, color, 'A');
    set_pix(6, 1, color, ' ');
    set_pix(7, 1, color, 'D');
    set_pix(8, 1, color, 'W');
    set_pix(9, 1, color, 'E');
    set_pix(10, 1, color, 'E');
    set_pix(11, 1, color, 'B');
}

