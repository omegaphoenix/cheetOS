#include "tokenStruct.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Dynamic constructor */
TokenGroup *TokenGroup_new(char **words,
                           token_type *tokens,
                           int num_tokens) {
    TokenGroup *new_token = malloc(sizeof(TokenGroup));
    if (new_token) {
        new_token->words = words;
        new_token->tokens = tokens;
        new_token->num_tokens = num_tokens;

        new_token->next_group = NULL;
        new_token->prev_group = NULL;
        return new_token;
    }
    fprintf(stderr, "Malloc failed for Token constructor.\n");
    return NULL;
}

/* Dynamic destructor */
void TokenGroup_free(TokenGroup *token) {
    free(token->words);
    free(token->tokens);
    free(token);
}



/* Initializes empty Token Linked List */
TokenGroupLList *TokenGroupLList_new() {
    TokenGroupLList *new_list = malloc(sizeof(TokenGroupLList));

    if (new_list) {
        new_list->first_group = NULL;
        new_list->last_group = NULL;

        new_list->num_groups = 0;
        return new_list;
    }
    else {
        fprintf(stderr, "Malloc failed for Token List constructor.\n");
        return NULL;
    }
}

/* Add to the end of the token linked list */
void token_group_llist_append(TokenGroupLList *token_groups,
                              TokenGroup *group) {
    TokenGroup *last_group = token_groups->last_group;

    if (last_group) {
        last_group->next_group = group;
        group->prev_group = last_group;

        token_groups->last_group = group;
    }
    else {
        token_groups->first_group = group;
        token_groups->last_group = group;
    }

    token_groups->num_groups++;
}

/* Dynamic destructor */
void TokenGroupLList_free(TokenGroupLList *token_groups) {
    TokenGroup *curr_group = token_groups->first_group;
    TokenGroup *temp_group;

    while (curr_group) {
        temp_group = curr_group->next_group;
        TokenGroup_free(curr_group);
        curr_group = temp_group;
    }

    free(token_groups);
}

/*
 * Helper function that truncates the array when a pipe is encountered.
 * It then adds the TokenGroup to the Linked List. Return true
 * if process finished successfully. Otherwise, returns false.
 */
bool truncate_and_add(TokenGroupLList *token_groups,
                      char **word_array,
                      token_type *token_type_array,
                      int size_of_group) {
    TokenGroup *new_token = NULL;
    char **truncated_words = NULL;
    token_type *truncated_tokens = NULL;

    truncated_words = realloc(word_array,
                                     size_of_group * sizeof(char *));

    truncated_tokens =
          realloc(token_type_array,
                  size_of_group * sizeof(token_type));

    if (!truncated_words || !truncated_tokens) {
      return false;
    }

    new_token = TokenGroup_new(truncated_words,
                               truncated_tokens,
                               size_of_group);

    if (!new_token) {
      return false;
    }

    token_group_llist_append(token_groups, new_token);
    return true;
}

/*
 * Takes the entire string of words, and encapsulates it
 * inside a token linked list delimited by pipes.
 */
TokenGroupLList *split_string_by_pipe(char **words,
                                      token_type *tokens,
                                      int array_size) {
    TokenGroupLList *new_list = NULL;
    char **word_array = NULL;
    token_type *token_type_array = NULL;

    int idx;
    int size_of_group = 0;
    bool add_to_list_successfully;

    new_list = TokenGroupLList_new();
    word_array = malloc(array_size * sizeof(char *));
    token_type_array = malloc(array_size * sizeof(token_type));

    if (new_list && word_array && token_type_array) {

        /* Iterating through array, and splitting when we see a pipe */
        for (idx = 0; idx < array_size; idx++) {
            char *word = words[idx];

            /* If it isn't a pipe, then add it to the group. */
            if (strcmp(word, "|") != 0) {
                word_array[size_of_group] = word;
                token_type_array[size_of_group] = tokens[idx];
                size_of_group++;

                /*
                 * End of array. This should always be a word,
                 * never a pipe.
                 */
                if (idx == array_size - 1) {
                    add_to_list_successfully =
                          truncate_and_add(new_list,
                                           word_array,
                                           token_type_array,
                                           size_of_group);
                    if (!add_to_list_successfully) {
                        goto error_exit;
                    }
                }
            }

            /* Pipe encountered */
            else {
                add_to_list_successfully = truncate_and_add(new_list,
                                                            word_array,
                                                            token_type_array,
                                                            size_of_group);
                if (!add_to_list_successfully) {
                    goto error_exit;
                }
                size_of_group = 0;
                word_array = malloc(array_size * sizeof(char *));
                token_type_array = malloc(array_size * sizeof(token_type));
            }
        }
        return new_list;
    }
    else {
      error_exit:
          fprintf(stderr,
              "Malloc Error in initialization of split_string_by_pipe.\n");
          free(word_array);
          free(token_type_array);
          TokenGroupLList_free(new_list);
          return NULL;
    }
}
