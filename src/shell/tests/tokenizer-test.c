#include "../tokenizer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Test basic 3 word command */
void parse_basicCommand_splitByWhitespace() {
    char *test_str = "a arg1 arg2";

    /* Expected results */
    int num_words = 3;
    char *expected[] = {"a", "arg1", "arg2"};
    token_type expected_tokens[] = {WORD, WORD, WORD};

    char **words = malloc(KiB(1));
    token_type *tokens = malloc(KiB(1) * sizeof(token_type));

    /* Call parse function to be tested */
    int res = parse_tokens(test_str, words, tokens);
    if (res != num_words) {
        fprintf(stderr, "Wrong number of tokens\n");
    }

    /* Check for correct split of words */
    for (int i = 0; i < num_words; i++) {
        if (strcmp(words[i], expected[i]) != 0) {
            fprintf(stderr, "Expected \"%s\" got \"%s\"\n", words[i], expected[i]);
        }
    }

    /* Verify tokens */
    for (int i = 0; i < num_words; i++) {
        if (tokens[i] != expected_tokens[i]) {
            fprintf(stderr, "Token %d doesn't match\n", i);
        }
    }
}

/* Test quote handling, whitespace, and redirect handling */
void parse_commandWithRedirectAndQuotes_splitByDelimiters() {
    char *test_str = "grep \"\\\"\" <logfile.txt";

    /* Expected results */
    int num_words = 4;
    char *expected[] = {"grep", "\"\\\"\"", "<", "logfile.txt"};
    token_type expected_tokens[] = {WORD, WORD, IN_REDIR, WORD};

    char **words = malloc(KiB(1));
    token_type *tokens = malloc(KiB(1) * sizeof(token_type));

    /* Call parse function to be tested */
    int res = parse_tokens(test_str, words, tokens);
    if (res != num_words) {
        fprintf(stderr, "Wrong number of tokens\n");
    }

    /* Check for correct split of words */
    for (int i = 0; i < num_words; i++) {
        if (strcmp(words[i], expected[i]) != 0) {
            fprintf(stderr, "Expected \"%s\" got \"%s\"\n", words[i], expected[i]);
        }
    }

    /* Verify tokens */
    for (int i = 0; i < num_words; i++) {
        if (tokens[i] != expected_tokens[i]) {
            fprintf(stderr, "Token %d doesn't match\n", i);
        }
    }
}

int main() {
    parse_basicCommand_splitByWhitespace();
    parse_commandWithRedirectAndQuotes_splitByDelimiters();
    return 0;
}

