#include "extCmd.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_cmd(char **args) {
    pid_t child_pid;
    int status;

    /* Fork off a child process to execute program */
    if ((child_pid = fork()) < 0) {
        fprintf(stderr, "Fork failed\n");
        exit(1);
    }
    if (child_pid == 0) {
        /* Execute command with child process */
        execvp(args[0], args);
        fprintf(stderr, "execvp failed - unknown command\n");
        exit(0);
    }
    else {
        /* Wait for child process to terminate */
        pid_t finished_pid = wait(&status);
        while (finished_pid != child_pid) {
            finished_pid = wait(&status);
        }

        return status;
    }
}
