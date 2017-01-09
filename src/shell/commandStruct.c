#include "commandStruct.h"
#include <stdlib.h>

/* * * * * * * * * * * * * * * * * * * * * * * * *
 *         Redirection struct functions          *
 * * * * * * * * * * * * * * * * * * * * * * * * */

/* Non dynamic constructor. */
Redirection Redirection_new(const token_type redirect_type,
                           char* redirect_location) {
  Redirection new_redirect;

  new_redirect.redirect_type = redirect_type;

  /* TODO: Make this deep later. */
  new_redirect.redirect_location = redirect_location;
  return new_redirect;
}

/* Dynamic constructor */
Redirection* Redirection_new_pointer(const token_type redirect_type,
                                     char* redirect_location) {
  Redirection* new_redirect = malloc(sizeof(Redirection));

  new_redirect->redirect_type = redirect_type;

  /* TODO: Make this deep later. */
  new_redirect->redirect_location = redirect_location;
  return new_redirect;
}

/* Non Dynamic destructor. Just frees the array */
void Redirection_free(Redirection* redirect_pointer) {
  /* TODO: Free the array */
}



/* Dynamic destructor. Frees both struct and array. */
void Redirection_free_pointer(Redirection* redirect_pointer) {
  /* TODO: Free the array before freeing the struct. */
  free(redirect_pointer);
}





/* * * * * * * * * * * * * * * * * * * * * * * * *
 *                Command structs                *
 * * * * * * * * * * * * * * * * * * * * * * * * */
/* Parsing will be done in these constructors. */

/* Non dynamic constructor */
Command Command_new(char** command_line, token_type** tokens) {
  Command new_command;

  return new_command;
}

/* Dynamic constructor */
Command* Command_new_pointer(char** command_line, token_type** tokens) {
  Command* new_command = malloc(sizeof(Command));

  return new_command;
}

/* Non dynamic destructor */
void Command_free(Command* command_pointer) {

}



/* Dynamic destructor */
void Command_free_pointer(Command* command_pointer) {

}

CommandLinkedList CommandLinkedList_new(Command* first_command) {
  CommandLinkedList command_LL;

  /* No need to deep copy here. */
  command_LL.first_command = first_command;

  /* TODO: find size. */
  return command_LL;
}

CommandLinkedList* CommandLinkedList_new_pointer(Command* first_command) {
  CommandLinkedList* command_LL = malloc(sizeof(CommandLinkedList));

  /* No need to deep copy here. */
  command_LL->first_command = first_command;

  /* TODO: find size. */
  return command_LL;
}