#ifndef COMMAND_STRUCT_H_
#define COMMAND_STRUCT_H_

#include "tokenizer.h"

/* To preserve the size of an array, yet also throw an error. */
#define COMMAND_PARSE_ERROR(x) ((int) -x)



/* * * * * * * * HELPER FUNCTIONS * * * * * * * */

/* 
 * This function will take in a command line, the appropriate
 * tokens, and output the size of the args list
 */
int filter_command_line_args(Command *command,
                             char **command_line,
                             char **args,
                             token_type *tokens,
                             int size_of_array);

/* Helper function that will just set attributes for a new command */
bool set_command_attributes(Command *command,
                            char **new_args,
                            char **old_args,
                            int filtered_size);



/* * * * * * * * * * * * * * * * * * * * * * * * *
 *               Redirection struct              *
 * * * * * * * * * * * * * * * * * * * * * * * * */

/* 
 * This struct will consider the different kinds of redirections
 * as well as the specific redirect location.
 */
typedef struct _Redirection {
  token_type redirect_type;
  char *redirect_location;
} Redirection;

/* Dynamic constructor */
Redirection* Redirection_new_pointer(const token_type redirect_type,
                                     char *redirect_location);

/* Dynamic destructor */
void Redirection_free_pointer(Redirection *redirect_pointer);



/* * * * * * * * * * * * * * * * * * * * * * * * *
 *                Command structs                *
 * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * This struct represents a shell command. It will store necessary
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

  /*
   * Array of arguments from command line.
   * Ex: For "grep Allow < input.txt > output.txt"
   *     args = ["grep", "Allow", NULL]
   */
  char **args;

  Command *next_command;
  Command *prev_command;
} Command;

/* Dynamic constructor */
Command *Command_new_pointer(char **command_line,
                             token_type *tokens,
                             int size_of_array);

/* Dynamic destructor */
void Command_free_pointer(Command *command_pointer);



/* * * * * * * * * * * * * * * * * * * * * * * * *
 *         Command Linked List Structs           *
 * * * * * * * * * * * * * * * * * * * * * * * * */

/* 
 * Double linked list of commands. For pipes, we will need information from
 * previous commands.
 */
typedef struct _CommandLinkedList {
  Command *first_command;
  Command *last_command;

  /* Size of the linked list. */
  int linked_list_size;
} CommandLinkedList;

/* Dynamic constructor. */
CommandLinkedList *CommandLinkedList_new_pointer();

/* This will append a command to the end of a linked list. */
void command_linked_list_append(CommandLinkedList *command_LL_pointer,
                                Command *command);

/* Dynamic destructor */
void CommandLinkedList_free_pointer(CommandLinkedList *command_LL_pointer);

#endif /* COMMAND_STRUCT_H_ */