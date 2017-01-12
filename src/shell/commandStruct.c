#include "commandStruct.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* * * * * * * * * * * * * * * * * * * * * * * * *
 *                Helper Functions               *
 * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * This function will take in a command line, the appropriate
 * tokens, set redirects and output the size of the args list
 */
int filter_command_line_args(Command *command,
                             char **command_line,
                             char **args,
                             token_type *tokens,
                             int size_of_array) {
  int iter;
  int filtered_idx = 0;
  char *command_word = NULL;

  /* Go through command, and parse out redirects. */
  for (iter = 0; iter < size_of_array; iter++) {
    switch (tokens[iter]) {

      /*
       * Empty statement after case, because you can't initialize variable
       * immediately after a label. Command word initialized above
       */
      case WORD: ;
        command_word = malloc((strlen(command_line[iter]) + 1));
        if (command_word) {
          strcpy(command_word, command_line[iter]);
          args[filtered_idx] = command_word;
          filtered_idx++;
          break;
        }
        else {
          return COMMAND_PARSE_ERROR(filtered_idx);
        }

     /*
      * Word after a redirect is the target. So we will increment
      * iter additionally here
      */
      case IN_REDIR:
      case OUT_REDIR: ;
        token_type redirect_type = tokens[iter];
        iter++;
        Redirection *new_redirect =
              Redirection_new_pointer(redirect_type, command_line[iter]);

        if (new_redirect) {
          if (new_redirect->redirect_type == IN_REDIR) {
            command->stdin_redirect = new_redirect;
          }
          else if (new_redirect->redirect_type == OUT_REDIR) {
            command->stdout_redirect = new_redirect;
          }
          else {
            command->stderr_redirect = new_redirect;
          }

          break;
        }
        else {
          return COMMAND_PARSE_ERROR(filtered_idx);
        }

      default:
        /* Must be a faulty token. */
        return COMMAND_PARSE_ERROR(filtered_idx);
    }
  }

  return filtered_idx;
}

/* Helper function that will just set attributes for a new command */
bool set_command_attributes(Command *command,
                            char **new_args,
                            char **old_args,
                            int filtered_size) {

  command->num_tokens = filtered_size;

  /* Reallocate args to have the appropriate size) */
  new_args = realloc(old_args, (filtered_size + 1) * sizeof(char*));

  if (new_args) {
    new_args[filtered_size] = NULL; /* Protocol to end array with NULL */
    command->args = new_args;
    return true;
  }
  else {
    return false;
  }
}



/* * * * * * * * * * * * * * * * * * * * * * * * *
 *         Redirection struct functions          *
 * * * * * * * * * * * * * * * * * * * * * * * * */

/* Dynamic constructor */
Redirection *Redirection_new_pointer(const token_type redirect_type,
                                     char *redirect_location) {
  Redirection *new_redirect = malloc(sizeof(Redirection));
  char *file_name = malloc((strlen(redirect_location) + 1));

  if (new_redirect && file_name) {
    new_redirect->redirect_type = redirect_type;

    strcpy(file_name, redirect_location);

    new_redirect->redirect_location = file_name;
    return new_redirect;
  }
  else {
    fprintf(stderr, "Malloc Redirection failed");
    free(file_name);
    free(new_redirect);
    return NULL;
  }

}

/* Dynamic destructor. Frees both struct and array. */
void Redirection_free_pointer(Redirection *redirect_pointer) {
  /* TODO: Free the array before freeing the struct. */
  if (redirect_pointer) {
    free(redirect_pointer->redirect_location);
  }

  free(redirect_pointer);
}



/* * * * * * * * * * * * * * * * * * * * * * * * *
 *                Command structs                *
 * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Dynamic constructor. Will only parse things before a pipe.
 * Thus, assume command_line will not contain a pipe.
 */
Command* Command_new_pointer(char **command_line,
                             token_type *tokens,
                             int size_of_array) {
  /*
   * To avoid a ton of if/else indentations,
   * I will do a few mallocs here first.
   */

  Command* new_command = NULL;
  char **args = NULL;

  new_command = malloc(sizeof(Command));
  args = malloc((size_of_array + 1) * sizeof(char *));

  /* All these are set to null so freeing would not be a problem */
  new_command->next_command = NULL;
  new_command->prev_command = NULL;
  new_command->stdin_redirect = NULL;
  new_command->stdout_redirect = NULL;
  new_command->stderr_redirect = NULL;

  char **new_args = NULL;
  int error_idx;
  int filtered_size;
  bool is_command_set;

  /* Handler for freeing everything before returning NULL */
  if (!args || !new_command) {
    goto error_exit;
  }

  filtered_size = filter_command_line_args(new_command,
                                               command_line,
                                               args,
                                               tokens,
                                               size_of_array);

  /*
   * An error value will be negative. We can use abs() to clear what
   * has been set before the error.
   */
  if (filtered_size < 0) {
    goto error_exit;
  }

  is_command_set =
        set_command_attributes(new_command, new_args, args, filtered_size);
  args = NULL; /* Allows for freeing in case of an error */

  if (is_command_set) {
    return new_command;
  }
  else {
    error_exit:
        fprintf(stderr, "Command Malloc Error or Tokenize parse error\n");
        for (error_idx = 0; error_idx < abs(filtered_size); error_idx++) {
          free(args[error_idx]);
        }
        free(new_args);
        free(args);
        free(new_command);

        return NULL;
  }
}

/* Dynamic destructor */
void Command_free_pointer(Command *command_pointer) {
  int idx;

  Redirection_free_pointer(command_pointer->stdin_redirect);
  Redirection_free_pointer(command_pointer->stdout_redirect);
  Redirection_free_pointer(command_pointer->stderr_redirect);

  /* Accounting for the null at the end of the args */
  for (idx = 0; idx < command_pointer->num_tokens + 1; idx++) {
    free(command_pointer->args[idx]);
  }
  free(command_pointer->args);
  free(command_pointer);
}



/* * * * * * * * * * * * * * * * * * * * * * * * *
 *         Command Linked List Structs           *
 * * * * * * * * * * * * * * * * * * * * * * * * */

/* Dynamic constructor. */
CommandLinkedList *CommandLinkedList_new_pointer() {
  CommandLinkedList *command_LL = malloc(sizeof(CommandLinkedList));

  command_LL->first_command = NULL;
  command_LL->last_command = NULL;
  command_LL->linked_list_size = 0;

  return command_LL;
}

/* Appends a node to the end of the linked list. */
void command_linked_list_append(CommandLinkedList *command_LL_pointer,
                                Command *command) {
  Command *last_command = command_LL_pointer->last_command;
  last_command->next_command = command;
  command->prev_command = last_command;

  linked_list_size++;
}

/* Dynamic destructor. Will free all Commands first before freeing itself */
void CommandLinkedList_free_pointer(CommandLinkedList *command_LL_pointer) {
  Command *curr_command = command_LL_pointer->first_command;
  Command *temp_command;

  /* Keep freeing until all nodes are freed */
  while (curr_command) {
    temp_command = curr_command->next_command;
    Command_free_pointer(curr_command);
    curr_command = temp_command;
  }

  free(command_LL_pointer);
}
