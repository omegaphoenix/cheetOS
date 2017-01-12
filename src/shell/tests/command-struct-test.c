#include "../commandStruct.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * *
 *        Redirection struct tests         *
 * * * * * * * * * * * * * * * * * * * * * */

/*
 * Constructor test; will first initialize expected attributes
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
 *           Helper Functions Tests        *
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

/* Helper to clean up a dynamically allocated char* array */
void free_command_and_args(Command *command,
                                 char **array_to_clean,
                                 int size_of_array) {
  int idx;
  for (idx = 0; idx < size_of_array; idx++) {
    free(array_to_clean[idx]);
  }
  free(array_to_clean);

  if (command) {
    if (command->stdin_redirect) {
      Redirection_free_pointer(command->stdin_redirect);
    }

    if (command->stdout_redirect) {
      Redirection_free_pointer(command->stdout_redirect);
    }

    if (command->stderr_redirect) {
      Redirection_free_pointer(command->stderr_redirect);
    }
  }

  free(command);
}

/* Tests the functionality of the filter_command_line_args function */
void filter_command_line_args_test() {
  Command *test_command = malloc(sizeof(Command));
  test_command->stdin_redirect = NULL;
  test_command->stdout_redirect = NULL;
  test_command->stderr_redirect = NULL;


  Redirection *expected_in = Redirection_new_pointer(IN_REDIR, "in.txt");
  Redirection *expected_out = Redirection_new_pointer(OUT_REDIR, "out.txt");
  char *test_command_line[] = {"grep", "test", "<", "in.txt", ">", "out.txt"};
  token_type input_tokens[] = {WORD, WORD, IN_REDIR, WORD, OUT_REDIR, WORD};
  char *expected_args[] = {"grep", "test"};
  int expected_size = 2;

  int size_of_array = 6;
  char **args = malloc(size_of_array * sizeof(char *));

  int filtered_size = filter_command_line_args(test_command,
                                               test_command_line,
                                               args,
                                               input_tokens,
                                               size_of_array);

  if (!expected_in || !expected_out || !test_command) {
    fprintf(stderr, "Constructor malloc failed.\n");
  }
  else if (filtered_size < 0) {
    fprintf(stderr, "Error occured inside filter_command_line_args.\n");
  }
  else if (filtered_size != expected_size) {
    fprintf(stderr, "Expected size did not equal actual filtered size.\n");
  }
  else if (strcmp(expected_args[0], args[0]) != 0 ||
           strcmp(expected_args[1], args[1]) != 0) {
    fprintf(stderr,
        "filter_command_line_args failed to parse command properly.\n");
  }
  else if (!Redirection_is_equal(expected_in, test_command->stdin_redirect) ||
      !Redirection_is_equal(expected_out, test_command->stdout_redirect)) {
    fprintf(stderr, "Command constructor sets incorrect Redirections.\n");
  }
  else {
    printf("filter_command_line_args successfully filters command line.\n");
  }

  Redirection_free_pointer(expected_in);
  Redirection_free_pointer(expected_out);
  free_command_and_args(test_command, args, abs(filtered_size));
}

/* Tests the functionality of the set_command_attributes function */
void set_command_attributes_test() {
  Command *test_command = malloc(sizeof(Command));

  int filtered_size = 2;
  char **new_args = NULL;
  char **old_args = malloc(filtered_size * sizeof(char *));
  old_args[0] = "grep";
  old_args[1] = "test";

  char *expected_words[] = {"grep", "test", NULL};


  int is_command_set = set_command_attributes(test_command,
                                              new_args,
                                              old_args,
                                              filtered_size);

  if (!test_command) {
    fprintf(stderr, "Malloc failed to allocate command struct.\n");
  }
  else if (!is_command_set) {
    fprintf(stderr, "Malloc failed to allocate new char* array.\n");
  }
  else if (test_command->num_tokens != filtered_size) {
    fprintf(stderr, "Failed to set correct size of array.\n");
  }
  else if (strcmp(expected_words[0], test_command->args[0]) != 0 ||
           strcmp(expected_words[1], test_command->args[1]) != 0 ||
           expected_words[2] || test_command->args[2]) {
    fprintf(stderr, "Failed to correctly set arguments for the command.\n");
  }
  else {
    printf("set_command_attributes successfully sets command attributes.\n");
  }

  free(new_args);
  free(test_command);
}

/* * * * * * * * * * * * * * * * * * * * * *
 *          Command struct tests           *
 * * * * * * * * * * * * * * * * * * * * * */

/*
 * Constructor test; will first initialize expected attributes
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
  printf("Testing Redirection Constructor...\n");
  Redirection_constructor_test();

  printf("\nTesting filter_command_line_args...\n");
  filter_command_line_args_test();

  printf("\nTesting set_command_attributes...\n");
  set_command_attributes_test();

  printf("\nTesting Command constructor...\n");
  Command_constructor_test();
  return 0;
}