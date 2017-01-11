#include "../tokenizer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Test basic 3 word command */
void parse_basicCommand_splitByWhitespace() {
    char *test_str = "a arg1 arg2";
    bool pass = true;

    /* Expected results */
    int num_words = 3;
    const char *expected[] = {"a", "arg1", "arg2"};
    const token_type expected_tokens[] = {WORD, WORD, WORD};

    char **words = malloc(KiB(1));
    token_type *tokens = malloc(KiB(1) * sizeof(token_type));
    if (!words || !tokens) {
        fprintf(stderr, "Malloc failed\n");
        pass = false;
    }

    /* Call parse function to be tested */
    int res = parse_tokens(test_str, words, tokens);
    if (res != num_words) {
        fprintf(stderr, "Wrong number of tokens\n");
        pass = false;
    }

    /* Check for correct split of words */
    int i;
    for (i = 0; i < num_words; i++) {
        if (strcmp(words[i], expected[i]) != 0) {
            fprintf(stderr, "Expected \"%s\" got \"%s\"\n", words[i],
                    expected[i]);
            pass = false;
        }
    }

    /* Verify tokens */
    for (i = 0; i < num_words; i++) {
        if (tokens[i] != expected_tokens[i]) {
            fprintf(stderr, "Token %d doesn't match\n", i);
            pass = false;
        }
    }

    if (pass) {
        printf("Tokenizes basic command split by whitespace correctly.\n");
    }
}

/* Test quote handling, whitespace, and redirect handling */
void parse_commandWithRedirectAndQuotes_splitByDelimiters() {
    /* Search for quotes within logfile.txt */
    char *test_str = "grep \"\\\"\" <logfile.txt";
    bool pass = true;

    /* Expected results */
    int num_words = 4;
    const char *expected[] = {"grep", "\"\\\"\"", "<", "logfile.txt"};
    const token_type expected_tokens[] = {WORD, WORD, IN_REDIR, WORD};

    char **words = malloc(KiB(1));
    token_type *tokens = malloc(KiB(1) * sizeof(token_type));
    if (!words || !tokens) {
        fprintf(stderr, "Malloc failed\n");
        pass = false;
    }

    /* Call parse function to be tested */
    int res = parse_tokens(test_str, words, tokens);
    if (res != num_words) {
        fprintf(stderr, "Wrong number of tokens\n");
        pass = false;
    }

    /* Check for correct split of words */
    int i;
    for (i = 0; i < num_words; i++) {
        if (strcmp(words[i], expected[i]) != 0) {
            fprintf(stderr, "Expected \"%s\" got \"%s\"\n", words[i],
                    expected[i]);
            pass = false;
        }
    }

    /* Verify tokens */
    for (i = 0; i < num_words; i++) {
        if (tokens[i] != expected_tokens[i]) {
            fprintf(stderr, "Token %d doesn't match\n", i);
            pass = false;
        }
    }

    if (pass) {
        printf("Tokenizes command with in redirect and quotes correctly.\n");
    }
}

void parse_commandWithPipeAndOutRedir_splitByDelimiters() {
    char *test_str = "grep \"Allow\" logfile.txt | sort > out.txt";
    bool pass = true;

    /* Expected results */
    int num_words = 7;
    const char *expected[] = {"grep", "\"Allow\"", "logfile.txt",
        "|", "sort", ">", "out.txt"};
    const token_type expected_tokens[] = {WORD, WORD, WORD, PIPE, WORD,
        OUT_REDIR, WORD};

    char **words = malloc(KiB(1));
    token_type *tokens = malloc(KiB(1) * sizeof(token_type));
    if (!words || !tokens) {
        fprintf(stderr, "Malloc failed\n");
        pass = false;
    }

    /* Call parse function to be tested */
    int res = parse_tokens(test_str, words, tokens);
    if (res != num_words) {
        fprintf(stderr, "Wrong number of tokens\n");
        pass = false;
    }

    /* Check for correct split of words */
    int i;
    for (i = 0; i < num_words; i++) {
        if (strcmp(words[i], expected[i]) != 0) {
            fprintf(stderr, "Expected \"%s\" got \"%s\"\n", words[i],
                    expected[i]);
            pass = false;
        }
    }

    /* Verify tokens */
    for (i = 0; i < num_words; i++) {
        if (tokens[i] != expected_tokens[i]) {
            fprintf(stderr, "Token %d doesn't match\n", i);
            pass = false;
        }
    }

    if (pass) {
        printf("Tokenizes command with out redirect and pipe correctly.\n");
    }
}

int main() {
    parse_basicCommand_splitByWhitespace();
    parse_commandWithRedirectAndQuotes_splitByDelimiters();
    parse_commandWithPipeAndOutRedir_splitByDelimiters();
    return 0;
}

