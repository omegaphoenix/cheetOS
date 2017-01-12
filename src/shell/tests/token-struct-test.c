#include "../tokenStruct.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void TokenGroup_constructor_test() {
  int num_tokens = 4;
  char *expected_words[] = {"grep", "test", "<", "test.txt"};
  token_type expected_tokens[] = {WORD, WORD, IN_REDIR, WORD};

  TokenGroup *new_group =
        TokenGroup_new(expected_words, expected_tokens, num_tokens);

  if (!new_group) {
    fprintf(stderr, "Token Constructor malloc failed.\n");
  }
  else if (new_group->tokens[0] != expected_tokens[0] ||
           new_group->tokens[1] != expected_tokens[1] ||
           new_group->tokens[2] != expected_tokens[2] ||
           new_group->tokens[3] != expected_tokens[3]) {
    fprintf(stderr, "Token Constructor failed to set tokens.\n");
  }
  else if (strcmp(new_group->words[0], expected_words[0]) != 0 ||
           strcmp(new_group->words[1], expected_words[1]) != 0 ||
           strcmp(new_group->words[2], expected_words[2]) != 0 ||
           strcmp(new_group->words[3], expected_words[3]) != 0 ) {
    fprintf(stderr, "Token Constructor failed to set words.\n");
  }
  else if (num_tokens != new_group->num_tokens) {
    fprintf(stderr, "Token Constructor failed to set token count.\n");
  }
  else {
    printf("Token Constructor successfully built from arguments.\n");
  }

  free(new_group);
}

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
    printf("TokenList Constructor produces empty list successfully.\n");
  }
  free(new_list);
}

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
    printf("Linked List append working for empty edge case.\n");
  }
  TokenGroupLList_free(new_list);
}

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
    printf("Linked List append function appending properly.\n");
  }

  TokenGroupLList_free(new_list);
}

int main() {
  printf("Testing TokenGroup Constructor...\n");
  TokenGroup_constructor_test();

  printf("\nTesting TokenGroupLList Constructor...\n");
  TokenGroupLList_constructor_test();

  printf("\nTesting token_group_llist_append edge case...\n");
  token_group_llist_empty_append_test();

  printf("\nTesting token_group_llist_append generic case...\n");
  token_group_llist_append_test();

  return 0;
}
