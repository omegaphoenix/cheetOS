#include "../commandStruct.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * *
 *        Redirection struct tests         *
 * * * * * * * * * * * * * * * * * * * * * */

/* Constructor test; will first initialize expected attributes
 * and test to see if the constructor contains them.
 */
void Redirection_constructor_test() {
  token_type expected_token = IN_REDIR;
  char *expected_file = "expected.txt";

  Redirection *test_redirect = \
            Redirection_new_pointer(expected_token, expected_file);

  if (!test_redirect) {
    fprintf(stderr, "Redirection malloc failed.\n");
  }
  else if (!test_redirect->redirect_type == expected_token) {
    fprintf(stderr, "Redirection constructor sets incorrect token type.\n");
  }

  else if (strcmp(expected_file, test_redirect->redirect_location) != 0) {
    fprintf(stderr, "Redirection constructor sets incorrect path.\n");
  }
  else {
    printf("Redirection constructor constructs correctly.\n");
  }

  Redirection_free_pointer(test_redirect);
}

/* * * * * * * * * * * * * * * * * * * * * *
 *          Command struct tests           *
 * * * * * * * * * * * * * * * * * * * * * */

/* Helper function to check if Redirection structs are equal */
bool Redirection_is_equal(Redirection *redirect_one,
                          Redirection *redirect_two) {
  if (!redirect_one->redirect_type == redirect_two->redirect_type) {
    return false;
  }
  else if (strcmp(redirect_one->redirect_location,
                  redirect_two->redirect_location) != 0) {
    return false;
  }
  return true;
}

/* Constructor test; will first initialize expected attributes
 * and test to see if the constructor contains them.
 */
void Command_constructor_test() {
  Redirection *expected_in = Redirection_new_pointer(IN_REDIR, "in.txt");
  Redirection *expected_out = Redirection_new_pointer(OUT_REDIR, "out.txt");
  int expected_tokens = 2;
  char *expected_words[] = {"grep", "test", NULL};

  char *input_command[] = {"grep", "test", "<", "in.txt", ">", "out.txt"};
  token_type input_tokens[] = {WORD, WORD, IN_REDIR, WORD, OUT_REDIR, WORD};
  int input_size = 6;

  Command *test_command = \
        Command_new_pointer(input_command, input_tokens, input_size);

  if (!expected_in || !expected_out || !test_command) {
    fprintf(stderr, "Constructor malloc failed.\n");
  }

  else if (!Redirection_is_equal(expected_in, test_command->stdin_redirect) ||
      !Redirection_is_equal(expected_out, test_command->stdout_redirect)) {
    fprintf(stderr, "Command constructor sets incorrect Redirections.\n");
  }

  else if (expected_tokens != test_command->num_tokens) {
    fprintf(stderr, "Command constructor incorrectly parses token array.\n");
  }

  else if (strcmp(expected_words[0], test_command->args[0]) != 0 ||
           strcmp(expected_words[1], test_command->args[1]) != 0 ||
           expected_words[2] || test_command->args[2]) {
    fprintf(stderr, "Command constructor doesn't generate correct args.\n");
  }
  else {
    printf("Command constructor constructs correctly.\n");
  }

  Redirection_free_pointer(expected_out);
  Redirection_free_pointer(expected_in);
  Command_free_pointer(test_command);
}

int main() {
  Redirection_constructor_test();
  Command_constructor_test();
  return 0;
}