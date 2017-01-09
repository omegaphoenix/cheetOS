#include <stdbool.h>

#define KiB(x) ((size_t) (x) << 10)

typedef enum {WORD, QUOTE, PIPE, IN_REDIR, OUT_REDIR, WHITE, END} token_type;

/*
 * Modify arrays to contain tokens. Return length of array.
 */
int parse(char *line, char **commands, token_type *tokens);

/* Return array of tokens */
token_type *tokenize(char *line);

/* Return token of current character */
token_type get_token(bool in_string, char curr_char);

/* Return true if this token indicates a new word or command */
bool is_delimeter(token_type token);

