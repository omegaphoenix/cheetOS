#include "builtin.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* 
* Internal cd command.
*/
int in_cd(char **args) {
  // If no argument is specified, change to home directory
  if (args[1] == NULL) { // TODO: check size of array/vector
    return chdir(getenv("HOME"));
  }
  else {
    return chdir(args[1]);
  }
  return 1;
}

/* 
* Internal exit command.
*/
int in_exit(char **args) {
  exit(0);
}
