#include "tokenizer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int parse_tokens(char *line, char **words, token_type *tokens) {
    size_t len = strlen(line);
    token_type *char_tokens = tokenize(line);

    /* Allocate memory for current word being built */
    char *curr_word = malloc(KiB(1));
    if (!curr_word) {
        fprintf(stderr, "Malloc failed\n");
    }
    int idx = 0;
    int curr_word_idx = 0;

    int i;
    for (i = 0; i < len; i++) {
        if (!is_delimiter(char_tokens[i])) {
            /* Save current character */
            if (char_tokens[i] != QUOTE) {
                curr_word[curr_word_idx] = line[i];
                curr_word_idx++;
            }
        }
        /* Handle whitespace, pipes, and redirects */
        else {
            /* Add finished word */
            if (curr_word_idx > 0) {
                curr_word[curr_word_idx] = '\0';
                words[idx] = curr_word;
                tokens[idx] = WORD;
                idx++;

                /* Allocate new memory for current word */
                curr_word = malloc(KiB(1));
                if (!curr_word) {
                    fprintf(stderr, "Malloc failed\n");
                }
                curr_word_idx = 0;
            }
            /* Add pipe or redirect operator */
            if (char_tokens[i] != WHITE) {
                /* Handle >> sequence */
                if (is_append_redir(char_tokens, i, len)) {
                    words[idx] = malloc(sizeof(char) + 1);
                    if (!words[idx]) {
                        fprintf(stderr, "Malloc failed\n");
                    }
                    words[idx][0] = line[i];
                    words[idx][1] = '\0';
                    tokens[idx] = AP_REDIR;
                    i++;
                }
                else {
                    /* Handle \0 at end of string */
                    words[idx] = malloc(sizeof(char) + 1);
                    if (!words[idx]) {
                        fprintf(stderr, "Malloc failed\n");
                    }
                    words[idx][1] = '\0';
                    words[idx][0] = line[i];
                    tokens[idx] = char_tokens[i];
                }
                idx++;
            }
        }
    }

    /* Add last word */
    if (curr_word_idx > 0) {
        curr_word[curr_word_idx] = '\0';
        words[idx] = curr_word;
        tokens[idx] = WORD;
        idx++;
        curr_word_idx = 0;
    }

    /* char_tokens is done being used. Free it here. */
    free(char_tokens);
    return idx;
}

token_type *tokenize(char *line) {
    size_t len = strlen(line);
    static token_type *tokens;
    tokens = malloc(len * sizeof(token_type));
    if (!tokens) {
        fprintf(stderr, "Malloc failed\n");
    }
    bool in_string = false;

    /* Parse line and label each character */
    int i;
    for (i = 0; i < len; i++) {
        char curr_char = line[i];
        token_type curr_token = get_token(in_string, curr_char);

        /* Handle tokens outside of quotes */
        if (!in_string) {
            if (curr_token == QUOTE) {
                in_string = true;
            }
            tokens[i] = curr_token;
        }
        /* Handle tokens within quotes */
        else {
            if (curr_token == QUOTE) {
                in_string = false;
            }
            /* Parse two characters when handling backslash */
            else if (curr_char == '\\') {
                tokens[i] = curr_token;
                i++;
                curr_char = line[i];
                curr_token = WORD;
            }
            tokens[i] = curr_token;
        }
    }

    return tokens;
}

token_type get_token(bool in_string, char curr_char) {
    if (curr_char == '\"') {
        return QUOTE;
    }
    /* If inside quotes, must be character */
    else if (in_string) {
        return WORD;
    }
    else {
        /* WORD represents a character within a word */
        switch (curr_char) {
            case '|': return PIPE;
            case '<': return IN_REDIR;
            case '>': return OUT_REDIR;
            case ' ': return WHITE;
            case '\t': return WHITE;
            default: return WORD;
        }
    }
}

bool is_delimiter(token_type token) {
    return token == PIPE || token == IN_REDIR ||
        token == OUT_REDIR || token == WHITE;
}

bool is_append_redir(token_type *tokens, int i, int len) {
    return i < len - 1 && tokens[i] == OUT_REDIR &&
        tokens[i + 1] == OUT_REDIR;
}
