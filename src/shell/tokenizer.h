#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

#define KiB(x) ((size_t) (x) << 10)

typedef enum {WORD, QUOTE, PIPE, IN_REDIR, OUT_REDIR, WHITE} token_type;

/*
 * Modify arrays to contain tokens. Return length of array.
 */
int parse_tokens(char *line, char **words, token_type *tokens);

/* Return array of tokens corresponding to characters */
token_type *tokenize(char *line);

/* Return token of current character */
token_type get_token(bool in_string, char curr_char);

/* Return true if this token indicates a new word or command */
bool is_delimiter(token_type token);

#endif /* TOKENIZER_H */
