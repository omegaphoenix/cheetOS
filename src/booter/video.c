#include "video.h"

#include "shooter.h"

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
#define COLOR_OFFSET 1

/* grid stores the character and color of each "pixel" on the screen*/
static short grid[GRID_HEIGHT][GRID_WIDTH];
static int bg_color;

char get_grid_color(int x, int y) {
    short grid_entry = grid[y][x];
    char color = grid_entry & 0xff;
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

void set_grid_pix(int x, int y, char color, char character) {
    /* Make sure color isn't too big */
    grid[y][x] = (color & 0xff) + (character << 8);
}

void clear() {
    int default_bg_color = BLACK;
    void *video = VIDEO_BUFFER;
    set_bg_color(default_bg_color);
}

void clear_bullet(Bullet bullet) {
    char color = get_color(BLACK, bg_color);
    set_grid_pix(bullet.x_pos, bullet.y_pos, color, ' ');
}

void clear_shooter(Shooter shooter) {
    int idx, x_pos, y_pos;
    char color = get_color(BLACK, bg_color);
    for (idx = 0; idx < 4; idx++) {
        x_pos = shooter.x_pos + (idx % 2);
        y_pos = shooter.y_pos + (idx / 2);
        set_grid_pix(x_pos, y_pos, color, ' ');
    }
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

void draw_shooter(Shooter actor) {
    int i, x, y;
    char c, color;
    if (actor.shooter_type == PLAYER) {
        color = get_color(WHITE, bg_color);
    }
    else {
        color = get_color(GREEN, bg_color);
    }
    for (i = 0; i < 4; i++) {
        x = actor.x_pos + (i % 2);
        y = actor.y_pos + (i / 2);
        c = actor.portrait[i];
        set_grid_pix(x, y, color, c);
    }
};

void draw_bullet(Bullet bullet) {
    int x_pos = bullet.x_pos;
    int y_pos = bullet.y_pos;
    char c = '.';
    char color = get_color(LIGHT_GRAY, bg_color);
    set_grid_pix(x_pos, y_pos, color, c);
}

void set_bg_color(char color) {
    bg_color = color;
}

void init_grid() {
    int i, x, y;
    char black = get_color(BLACK, BLACK);
    for (i = 0; i < GRID_HEIGHT * GRID_WIDTH; i++) {
        x = i % GRID_WIDTH;
        y = i / GRID_WIDTH;
        set_grid_pix(x, y, black, ' ');
    }
}

void init_video(void) {
    /* TODO:  Do any video display initialization you might want to do, such
     *        as clearing the screen, initializing static variable state, etc.
     */
    init_grid();
    init_score();
}

void update_score(int score) {
    init_score();
    int new_score = score;
    int pow_of_ten = 1;
    int length = 13;
    char color = get_color(CYAN, BLACK);
    if (new_score > MAX_SCORE) {
        new_score = MAX_SCORE;
    }
    int i;
    for (i = 0; i < MAX_DIGITS; i++) {
        int curr_dig = new_score / pow_of_ten;
        curr_dig = curr_dig % 10;
        char character = curr_dig + '0';
        set_grid_pix(length - i, GRID_HEIGHT - 1, color, character);
        pow_of_ten *= 10;
    }
}

void init_score() {
    int i;
    int y = GRID_HEIGHT - 1;
    char *word = "SCORE: 000000";
    int length = 13;
    char color = get_color(CYAN, BLACK);
    for (i = 0; i < length; i++) {
        set_grid_pix(i + 1, GRID_HEIGHT - 1, color, word[i]);
    }
}

void game_over(int message) {
    int num_messages = 2;
    set_bg_color(BLACK);
    char color = get_color(MAGENTA, bg_color);
    char *word;
    char length;
    switch (message % num_messages) {
        case 0:
            word = "GAME OVER DWEEB";
            length = 15;
            break;
        case 1:
            word = "BETTER LUCK NEXT TIME";
            length = 21;
            break;
        case 2:
            word = "YO MAMA SHOOTS BETTER THAN YOU";
            length = 30;
            break;
        default:
            word = "GAME OVER";
            length = 9;
            break;
    }
    int i;
    for (i = 0; i < length; i++) {
        set_grid_pix(i, GRID_HEIGHT / 2, color, word[i]);
    }
}
