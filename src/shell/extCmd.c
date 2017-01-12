#include "extCmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void execute_cmd(char **args) {
    pid_t pid;
    /* Fork off a child process to execute program */
    if ((pid = fork()) < 0) {
        fprintf(stderr, "Fork failed\n");
        exit(1);
    }
    /* Wait for child process to terminate */
}
