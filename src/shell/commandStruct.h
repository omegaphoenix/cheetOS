#ifndef COMMAND_STRUCT_H_
#define COMMAND_STRUCT_H_

#include "tokenizer.h"
/* * * * * * * * * * * * * * * * * * * * * * * * *
 *               Redirection struct              *
 * * * * * * * * * * * * * * * * * * * * * * * * */

/* This struct will consider the different kinds of redirections
 * as well as the specific redirect location.
 */
typedef struct _Redirection {
  token_type redirect_type;
  char *redirect_location;
} Redirection;

/* Non dynamic constructor. */
Redirection Redirection_new(const token_type redirect_type,
                            char *redirect_location);

/* Dynamic constructor */
Redirection* Redirection_new_pointer(const token_type redirect_type,
                                     char *redirect_location);

/* Non dynamic destructor */
void Redirection_free(Redirection *redirect_pointer);

/* Dynamic destructor */
void Redirection_free_pointer(Redirection *redirect_pointer);





/* * * * * * * * * * * * * * * * * * * * * * * * *
 *                Command structs                *
 * * * * * * * * * * * * * * * * * * * * * * * * */

/* This struct represents a shell command. It will store necessary
 * information such as redirection and token numbers.
 */
typedef struct _Command Command;
typedef struct _Command {
  /* Keeps track of all redirections in the command (input, output, error) */
  Redirection *stdin_redirect;
  Redirection *stdout_redirect;
  Redirection *stderr_redirect;

  /* Number of tokens for a particular command */
  int num_tokens;

  /* Array of arguments from command line. */
  char **args;

  Command *next_command;
  Command *prev_command;
} Command;

/* Non dynamic constructor */
Command Command_new(char **command_line, token_type *tokens, int size_of_array);

/* Dynamic constructor */
Command *Command_new_pointer(char **command_line, token_type *tokens, int size_of_array);

/* Non dynamic destructor */
void Command_free(Command *command_pointer);

/* Dynamic destructor */
void Command_free_pointer(Command *command_pointer);





/* Double linked list of commands. For pipes, we will need information from
 * previous commands.
 */
typedef struct _CommandLinkedList {
  Command *first_command;

  /* Size of the linked list. */
  int linked_list_size;
} CommandLinkedList;

/* Non dynamic constructor */
CommandLinkedList CommandLinkedList_new(Command *first_command);

/* Dynamic constructor. */
CommandLinkedList *CommandLinkedList_new_pointer(Command *first_command);

/* Non dynamic destructor */
void CommandLinkedList_free(CommandLinkedList *command_LL_pointer);

/* Dynamic destructor */
void CommandLinkedList_free_pointer(CommandLinkedList *command_LL_pointer);

#endif /* COMMAND_STRUCT_H_ */