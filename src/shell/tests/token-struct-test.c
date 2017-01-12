#include "../tokenStruct.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper function to compare array of token_types */
bool compare_tokens(token_type *token_array1,
                    token_type *token_array2,
                    int size_of_array) {
    int idx;
    for (idx = 0; idx < size_of_array; idx++) {
        if (token_array1[idx] != token_array2[idx]) {
            return false;
        }
    }
    return true;
}

/* Helper function to compare array of char pointers */
bool compare_words(char **words_array1,
                   char **words_array2,
                   int size_of_array) {
    int idx;
    for (idx = 0; idx < size_of_array; idx++) {
        if (strcmp(words_array1[idx], words_array2[idx]) != 0) {
            return false;
        }
    }
    return true;
}

/* Helper function to compare a TokenGroup node with expected values */
bool compare_token_group(TokenGroup *node,
                         char **expected_words,
                         token_type *expected_tokens,
                         int expected_size) {
    return (node->num_tokens == expected_size &&
            compare_tokens(node->tokens,
                           expected_tokens,
                           expected_size) &&
            compare_words(node->words,
                           expected_words,
                           expected_size));
}

/* Testing struct constructor */
void TokenGroup_constructor_test() {
    int test_size = 4;
    char *expected_words[] = {"grep", "test", "<", "test.txt"};
    token_type expected_tokens[] = {WORD, WORD, IN_REDIR, WORD};

    TokenGroup *new_group =
        TokenGroup_new(expected_words, expected_tokens, test_size);

    if (!new_group) {
        fprintf(stderr, "Token Constructor malloc failed.\n");
    }
    else if (!compare_tokens(expected_tokens, new_group->tokens, test_size)) {
        fprintf(stderr, "Token Constructor failed to set tokens.\n");
    }
    else if (!compare_words(expected_words, new_group->words, test_size)) {
        fprintf(stderr, "Token Constructor failed to set words.\n");
    }
    else if (test_size != new_group->num_tokens) {
        fprintf(stderr, "Token Constructor failed to set token count.\n");
    }
    else {
        printf("SUCCESS.\n");
    }

    free(new_group);
}

/* Testing struct constructor */
void TokenGroupLList_constructor_test() {
    TokenGroupLList *new_list = TokenGroupLList_new();
    if (!new_list) {
        fprintf(stderr, "TokenList Constructor malloc failed.\n");
    }
    else if (new_list->first_group ||
             new_list->last_group ||
             new_list->num_groups != 0) {
        fprintf(stderr, "TokenList should be empty.\n");
    }
    else {
        printf("SUCCESS.\n");
    }
    free(new_list);
}

/* Testing Linked List append with initial empty list */
void token_group_llist_empty_append_test() {
    TokenGroup *test_group = TokenGroup_new(NULL, NULL, 0);

    TokenGroupLList *new_list = TokenGroupLList_new();

    if (!new_list || !test_group) {
        fprintf(stderr, "TokenList Constructor malloc failed.\n");
    }

    token_group_llist_append(new_list, test_group);

    if (new_list->first_group != new_list->last_group ||
            !new_list->first_group || new_list->num_groups != 1) {
        fprintf(stderr, "TokenList singleton not set properly.\n");
    }
    else {
        printf("SUCCESS.\n");
    }
    TokenGroupLList_free(new_list);
}

/* Testing Linked List append with a non-empty list. */
void token_group_llist_append_test() {
    TokenGroup *test_group = TokenGroup_new(NULL, NULL, 0);
    TokenGroup *test_group_2 = TokenGroup_new(NULL, NULL, 1);

    TokenGroupLList *new_list = TokenGroupLList_new();

    if (!new_list || !test_group || !test_group_2) {
        fprintf(stderr, "TokenList Constructor malloc failed.\n");
    }

    token_group_llist_append(new_list, test_group);
    token_group_llist_append(new_list, test_group_2);

    if (new_list->num_groups != 2) {
        fprintf(stderr, "TokenList append not appending properly.\n");
    }
    else if (new_list->first_group->num_tokens != 0) {
        fprintf(stderr, "Linked List head not set properly.\n");
    }
    else if (new_list->last_group->num_tokens != 1) {
        fprintf(stderr, "Linked List tail not set properly.\n");
    }
    else if (new_list->first_group->next_group->num_tokens != 1) {
        fprintf(stderr, "Group next pointer not set properly.\n");
    }
    else if (new_list->last_group->prev_group->num_tokens != 0) {
        fprintf(stderr, "Group prev pointer not set properly.\n");
    }
    else {
        printf("SUCCESS.\n");
    }

    TokenGroupLList_free(new_list);
}

/* Testing pipe delimitation with no pipes */
void split_string_by_pipe_no_pipe_tests() {
    char *input_command[] = {"grep", "test", "<", "in.txt", ">", "out.txt"};
    token_type input_tokens[] = {WORD, WORD, IN_REDIR, WORD, OUT_REDIR, WORD};
    int input_size = 6;
    int expected_num_groups = 1; /* no pipes */

    TokenGroupLList *new_list =
        split_string_by_pipe(input_command, input_tokens, input_size);

    if (!new_list) {
        fprintf(stderr, "Token List not malloced properly.");
    }
    else if (!new_list->first_group || !new_list->last_group) {
        fprintf(stderr, "Token List failed to set head and tail.\n");
    }
    else if (new_list->num_groups != expected_num_groups) {
        fprintf(stderr, "Token List has inaccurate number of groups.\n");
    }
    else if (!compare_token_group(new_list->first_group,
                                  input_command,
                                  input_tokens,
                                  input_size)) {
        fprintf(stderr, "Token List failed to properly set node.\n");
    }
    else {
        printf("SUCCESS.\n");
    }

    TokenGroupLList_free(new_list);
}

/*
 * Testing pipe delimitation with pipes. Using example input from
 * assignment.
 */
void split_string_by_pipe_with_pipe_tests() {
    char *input_command[] =
        { "grep", "Allow", "<", "logfile.txt", "|", "grep", "-v", "google",
          "|", "sort", "|", "uniq", "-c", ">", "out.txt" };

    token_type input_tokens[] =
        { WORD, WORD, IN_REDIR, WORD, PIPE, WORD, WORD, WORD, PIPE, WORD,
          PIPE, WORD, WORD, OUT_REDIR, WORD };
    int input_size = 15;
    int expected_num_groups = 4; /* 3 pipes */

    /* Expected results */
    char *group1_words[] = {"grep", "Allow", "<", "logfile.txt"};
    token_type group1_tokens[] = {WORD, WORD, IN_REDIR, WORD};
    int group1_size = 4;

    char *group2_words[] = {"grep", "-v", "google"};
    token_type group2_tokens[] = {WORD, WORD, WORD};
    int group2_size = 3;

    char *group3_words[] = {"sort"};
    token_type group3_tokens[] = {WORD};
    int group3_size = 1;

    char *group4_words[] = {"uniq", "-c", ">", "out.txt"};
    token_type group4_tokens[] = {WORD, WORD, OUT_REDIR, WORD};
    int group4_size = 4;

    TokenGroupLList *new_list =
        split_string_by_pipe(input_command, input_tokens, input_size);

    /* Verify TokenGroupLList values */
    if (!new_list) {
        fprintf(stderr, "Token List not malloced properly.");
    }
    else if (new_list->num_groups != expected_num_groups) {
        fprintf(stderr, "Token List has inaccurate number of groups.\n");
    }
    else if (!new_list->first_group || !new_list->last_group) {
        fprintf(stderr, "Token List failed to set head and tail.\n");
    }
    /* Looking through each of the nodes now. */
    else if (!compare_token_group(new_list->first_group,
                                  group1_words,
                                  group1_tokens,
                                  group1_size)) {
        fprintf(stderr, "First node incorrectly parsed.\n");
    }
    else if (!compare_token_group(new_list->first_group->next_group,
                                  group2_words,
                                  group2_tokens,
                                  group2_size)) {
        fprintf(stderr, "Second node incorrectly parsed.\n");
    }
    else if (!compare_token_group(new_list->last_group->prev_group,
                                  group3_words,
                                  group3_tokens,
                                  group3_size)) {
        fprintf(stderr, "Third node incorrectly parsed.\n");
    }
    else if (!compare_token_group(new_list->last_group,
                                  group4_words,
                                  group4_tokens,
                                  group4_size)) {
        fprintf(stderr, "Fourth node incorrectly parsed.\n");
    }
    else {
        printf("SUCCESS.\n");
    }

    TokenGroupLList_free(new_list);
}

int main() {
    printf("Testing TokenGroup Constructor...\n");
    TokenGroup_constructor_test();

    printf("\nTesting TokenGroupLList Constructor...\n");
    TokenGroupLList_constructor_test();

    printf("\nTesting token_group_llist_append EMPTY edge case...\n");
    token_group_llist_empty_append_test();

    printf("\nTesting token_group_llist_append generic case...\n");
    token_group_llist_append_test();

    printf("\nTesting split_string_by_pipe NO PIPE case...\n");
    split_string_by_pipe_no_pipe_tests();

    printf("\nTesting split_string_by_pipe WITH PIPES case...\n");
    split_string_by_pipe_with_pipe_tests();

    return 0;
}
