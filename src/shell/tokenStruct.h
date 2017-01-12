#ifndef TOKEN_STRUCT_H_
#define TOKEN_STRUCT_H_

#include "tokenizer.h"

#include <stdio.h>

/*
 * This struct contains the words and token_types for each
 * corresponding node.
 */
typedef struct _TokenGroup TokenGroup;
typedef struct _TokenGroup {
    char **words;
    token_type *tokens;
    int num_tokens;

    TokenGroup *next_group;
    TokenGroup *prev_group;
} TokenGroup;

/* Dynamic constructor */
TokenGroup *TokenGroup_new(char **words, token_type *tokens, int num_tokens);

/* Dynamic destructor */
void TokenGroup_free(TokenGroup *token);



typedef struct _TokenGroupLList {
    TokenGroup *first_group;
    TokenGroup *last_group;

    int num_groups;
}   TokenGroupLList;

/* Initializes empty Token Linked List */
TokenGroupLList *TokenGroupLList_new();

/* Add to the end of the token linked list */
void token_group_llist_append(TokenGroupLList *token_groups,
                              TokenGroup *group);
/* Dynamic destructor */
void TokenGroupLList_free(TokenGroupLList *token_groups);


/*
 * Takes the entire string of words, and encapsulates it
 * inside a token linked list delimited by pipes.
 */
TokenGroupLList *split_string_by_pipe(char **words,
                                      token_type *tokens,
                                      int array_size);

#endif /* TOKEN_STRUCT_H_ */
