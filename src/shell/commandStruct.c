#include "commandStruct.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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
/* Dynamic constructor. Will only parse things before a pipe.
 * Thus, assume command_line will not contain a pipe.
 */
Command* Command_new_pointer(char **command_line,
                             token_type *tokens,
                             int size_of_array) {
  /* To avoid a ton of if/else indentations,
   * I will do a few mallocs here first.
   */
  Command* new_command = malloc(sizeof(Command));
  char **args = malloc((size_of_array + 1) * sizeof(char *));
  char *command_word = NULL;
  char **new_args = NULL;
  int filtered_idx = 0;
  int error_idx;
  int iter;

  /* Handler for freeing everything before returning NULL */
  if (!args || !new_command) {
    error_exit:
      fprintf(stderr, "Command Malloc Error or Tokenize parse error\n");
      for (error_idx = 0; error_idx < filtered_idx; error_idx++) {
        free(args[error_idx]);
      }
      free(new_args);
      free(command_word);
      free(args);
      free(new_command);

      return NULL;
  }

  /* All these are set to null so freeing would not be a problem */
  new_command->next_command = NULL;
  new_command->prev_command = NULL;
  new_command->stdin_redirect = NULL;
  new_command->stdout_redirect = NULL;
  new_command->stderr_redirect = NULL;

  /* Go through command, and parse out redirects. */
  for (iter = 0; iter < size_of_array; iter++) {
    switch (tokens[iter]) {

      /* Empty statement after case, because you can't initialize variable 
       * immediately after a label
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
          goto error_exit;
        }

     /* Word after a redirect is the target. So we will increment 
      * iter additionally here
      */
      case IN_REDIR:
      case OUT_REDIR: ;
        token_type redirect_type = tokens[iter];
        iter++;
        Redirection *new_redirect = \
              Redirection_new_pointer(redirect_type, command_line[iter]);

        if (new_redirect) {
          if (new_redirect->redirect_type == IN_REDIR) {
            new_command->stdin_redirect = new_redirect;
          }
          else if (new_redirect->redirect_type == OUT_REDIR) {
            new_command->stdout_redirect = new_redirect;
          }
          else {
            new_command->stderr_redirect = new_redirect;
          }

          break;
        }
        else {
          goto error_exit;
        }

      default:
        /* Must be a faulty token. */
        goto error_exit;
    }
  }

  new_command->num_tokens = filtered_idx;

  /* Reallocate args to have the appropriate size) */
  new_args = realloc(args, (filtered_idx + 1) * sizeof(char*));
  args = NULL;
  if (new_args) {
    new_args[filtered_idx] = NULL; /* Protocol to end array with NULL */
    new_command->args = new_args;
  }
  else {
    goto error_exit;
  }

  return new_command;
}

/* Non dynamic destructor */
void Command_free(Command *command_pointer) {

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


CommandLinkedList CommandLinkedList_new(Command *first_command) {
  CommandLinkedList command_LL;

  /* No need to deep copy here. */
  command_LL.first_command = first_command;

  /* TODO: find size. */
  return command_LL;
}

CommandLinkedList *CommandLinkedList_new_pointer(Command *first_command) {
  CommandLinkedList *command_LL = malloc(sizeof(CommandLinkedList));

  /* No need to deep copy here. */
  command_LL->first_command = first_command;

  /* TODO: find size. */
  return command_LL;
}