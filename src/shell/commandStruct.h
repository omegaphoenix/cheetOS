#ifndef COMMAND_STRUCT_H_
#define COMMAND_STRUCT_H_

/* * * * * * * * * * * * * * * * * * * * * * * * *
 *               Redirection struct              *
 * * * * * * * * * * * * * * * * * * * * * * * * */

// This struct will consider the different kinds of redirections as well as the
// specific redirect location.
struct Redirection
{
  token_type redirectType;
  char* redirectLocation;
}

// Non dynamic constructor.
Redirection redirectionNew(const token_type redirectType,
                           const char* redirectLocation);

// Dynamic constructor
Redirection* redirectionNewPointer(const token_type redirectType,
                                   const char* redirectLocation);

// Non dynamic destructor
void freeRedirection(Redirection redirectPointer);

// Dynamic destructor
void freeRedirectionPointer(Redirection* redirectPointer);





/* * * * * * * * * * * * * * * * * * * * * * * * *
 *                Command structs                *
 * * * * * * * * * * * * * * * * * * * * * * * * */

// This struct represents a shell command. It will store necessary information
// such as redirection and token numbers.
struct Command
{
  // Keeps track of all redirections in the command (input, output, error)
  Redirection** redirects;

  // Number of tokens for a particular command
  int numTokens;

  // Array of arguments from command line.
  char** args;

  Command* nextCommand;
  Command* prevCommand;
};

// Non dynamic constructor
Command commandNew(char** commandLine, token_type** tokens);

// Dynamic constructor
Command* commandNewPointer(char** commandLine, token_type** tokens);

// Non dynamic destructor
void freeCommand(Command commandPointer);

// Dynamic destructor
void freeCommandPointer(Command* commandPointer);





// Double linked list of commands. For pipes, we will need information from
// previous commands.
struct CommandLinkedList
{
  Command* firstCommand;

  // Size of the linked list.
  int linkedListSize;
}

// Non dynamic constructor
CommandLinkedList commandLinkedListNew(const Command* firstCommand);

// Dynamic constructor.
CommandLinkedList* commandLinkedListNewPointer(const Command* firstCommand);

// Non dynamic destructor
void freeCommandLinkedList(CommandLinkedList commandLL);

// Dynamic destructor
void freeCommandLinkedListPointer(CommandLinkedList* commandLLPointer);

#endif //COMMAND_STRUCT_H_