#include <stdbool.h>

typedef enum {CHAR, QUOTE, PIPE, REDIRECT, WHITE} token_type;

token_type get_token(bool in_string, char curr_char);
token_type *tokenize(char *line);
