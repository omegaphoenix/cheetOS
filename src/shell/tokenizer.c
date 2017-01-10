#include "tokenizer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

int parse_tokens(char *line, char **words, token_type *tokens) {
  size_t len = strlen(line);
  token_type *char_tokens = tokenize(line);
  char *curr_word = malloc(KiB(1));
  int idx, curr_word_idx = 0;

  for (int i = 0; i < len; i++) {
    if (!is_delimiter(char_tokens[i])) {
      /* Save current character */
      curr_word[curr_word_idx] = line[i];
      curr_word_idx++;
    }
    /* Handle whitespace, pipes, and redirects */
    else {
      /* Add finished word */
      if (curr_word_idx > 0) {
        words[idx] = curr_word;
        tokens[idx] = WORD;
        idx++;
        curr_word_idx = 0;
        curr_word = malloc(KiB(1));
      }
      /* Add pipe or redirect operator */
      if (char_tokens[i] != WHITE) {
        words[idx] = malloc(sizeof(char));
        words[idx][0] = line[i];
        tokens[idx] = char_tokens[i];
        idx++;
      }
    }
  }

  /* Add last word */
  if (curr_word_idx > 0) {
    words[idx] = curr_word;
    tokens[idx] = WORD;
    idx++;
    curr_word_idx = 0;
  }

  /* Indicate end of arrays */
  words[idx] = malloc(sizeof(char));
  words[idx][0] = ' ';
  tokens[idx] = END;

  return idx;
}

token_type *tokenize(char *line) {
  size_t len = strlen(line);
  static token_type *tokens;
  tokens = malloc(len * sizeof(token_type));
  bool in_string = false;

  for (int i = 0; i < len; i++) {
    char curr_char = line[i];
    token_type curr_token = get_token(in_string, curr_char);
    if (!in_string) {
      if (curr_token == QUOTE) {
        in_string = true;
      }
      tokens[i] = curr_token;
    }
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
  else if (curr_char == '|') {
    return PIPE;
  }
  else if (curr_char == '<') {
    return IN_REDIR;
  }
  else if (curr_char == '>') {
    return OUT_REDIR;
  }
  else if (curr_char == ' ' || curr_char == '\t') {
    return WHITE;
  }
  else {
    return WORD;
  }
}

bool is_delimiter(token_type token) {
  return token == PIPE || token == IN_REDIR ||
    token == OUT_REDIR || token == WHITE;
}
