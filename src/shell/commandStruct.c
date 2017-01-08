#include "commandStruct.h"

/* * * * * * * * * * * * * * * * * * * * * * * * *
 *         Redirection struct functions          *
 * * * * * * * * * * * * * * * * * * * * * * * * */

// Non dynamic constructor.
Redirection redirectionNew(const token_type redirectType,
                           const char* redirectLocation)
{
  Redirection newRedirect;

  newRedirect.redirectType = redirectType;
  newRedirect.redirectLocation = redirectLocation; //TODO: Make this deep later.
  return newRedirect;
}

// Dynamic constructor
Redirection* redirectionNewPointer(const token_type redirectType,
                                   const char* redirectLocation)
{
  Redirection* newRedirect = malloc(sizeof(Redirection));

  newRedirect->redirectType = redirectType;
  newRedirect->redirectLocation = redirectLocation; //TODO: Make this deep later.
  return newRedirect;
}

// Dynamic destructor
void freeRedirection(Redirection redirectPointer)
{
  // TODO: Free the array
}



// Dynamic destructor
void freeRedirectionPointer(Redirection* redirectPointer)
{
  // TODO: Free the array before freeing the struct.
  free(redirectPointer);
}





/* * * * * * * * * * * * * * * * * * * * * * * * *
 *                Command structs                *
 * * * * * * * * * * * * * * * * * * * * * * * * */
// Parsing will be done in these constructors.

// Non dynamic constructor
Command commandNew(char** commandLine, token_type** tokens)
{

}

// Dynamic constructor
Command* commandNewPointer(char** commandLine, token_type** tokens)
{

}

// Non dynamic destructor
void freeCommand(Command commandPointer)
{

}



// Dynamic destructor
void freeCommandPointer(const Command* commandPointer)
{

}

CommandLinkedList commandLinkedListNew(const Command* firstCommand)
{
  CommandLinkedList commandLL;

  // No need to deep copy here.
  commandLL.firstCommand = firstCommand;

  //TODO: find size.
}

CommandLinkedList* commandLinkedListNewPointer(const Command* firstCommand)
{
  CommandLinkedList* commandLL = malloc(sizeof(CommandLinkedList));

  // No need to deep copy here.
  commandLL->firstCommand = firstCommand;

  // TODO: find size.
}