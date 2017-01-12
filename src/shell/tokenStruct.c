#include "tokenStruct.h"

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
