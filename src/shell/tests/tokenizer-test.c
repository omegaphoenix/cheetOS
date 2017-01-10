#include "../tokenizer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void parse_basicCommand_splitByWhitespace() {
  char *test_str = "a arg1 arg2";
  char **words = malloc(KiB(1));
  token_type *tokens = malloc(KiB(1) * sizeof(token_type));

  int res = parse_tokens(test_str, words, tokens);
  if (res != 3) {
    fprintf(stderr, "Wrong number of tokens\n");
  }

  if (strcmp(words[0], "a") != 0) {
    fprintf(stderr, "Expected \"a\" got \"%s\"\n", words[0]);
  }

  if (strcmp(words[1], "arg1") != 0) {
    fprintf(stderr, "Expected \"arg1\" got \"%s\"\n", words[1]);
  }

  if (strcmp(words[2], "arg2") != 0) {
    fprintf(stderr, "Expected \"arg2\" got \"%s\"\n", words[2]);
  }

  if (tokens[0] != tokens[1] || tokens[1] != tokens[2]
      || tokens[2] != WORD) {
    fprintf(stderr, "Wrong token");
  }
}

void parse_commandWithRedirectAndQuotes_splitByDelimiters() {
  char *test_str = "grep \"\\\"\"<logfile.txt";
  char **words = malloc(KiB(1));
  token_type *tokens = malloc(KiB(1) * sizeof(token_type));

  int res = parse_tokens(test_str, words, tokens);
  if (res != 4) {
    fprintf(stderr, "Wrong number of tokens\n");
  }

  if (strcmp(words[0], "grep") != 0) {
    fprintf(stderr, "Expected \"a\" got \"%s\"\n", words[0]);
  }

  if (strcmp(words[1], "\"\\\"\"") != 0) {
    fprintf(stderr, "Expected \"\"\\\"\"\" got \"%s\"\n", words[1]);
  }

  if (strcmp(words[2], "<") != 0) {
    fprintf(stderr, "Expected \"arg2\" got \"%s\"\n", words[2]);
  }

  if (strcmp(words[3], "logfile.txt") != 0) {
    fprintf(stderr, "Expected \"logfile.txt\" got \"%s\"\n", words[2]);
  }

  if (tokens[0] != tokens[1] || tokens[1] != tokens[3]
      || tokens[3] != WORD || tokens[2] != IN_REDIR) {
    fprintf(stderr, "Wrong token");
  }
}

int main() {
  parse_basicCommand_splitByWhitespace();
  parse_commandWithRedirectAndQuotes_splitByDelimiters();
  return 0;
}

