#include "extCmd.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_cmd(char **argv) {
    pid_t child_pid;
    int status;

    /* Fork off a child process to execute program */
    if ((child_pid = fork()) < 0) {
        fprintf(stderr, "Fork failed\n");
        exit(1);
    }

    /* Child process */
    if (child_pid == 0) {
        /* Execute command with child process */
        execvp(argv[0], argv);
        fprintf(stderr, "execvp failed - unknown command\n");
        exit(0);
    }

    /* Parent process */
    else {
        /* Wait for child process to terminate */
        pid_t finished_pid = wait(&status);
        while (finished_pid != child_pid) {
            finished_pid = wait(&status);
        }

        return status;
    }
}

int execute_ext_cmd(int argc, char **argv) {
    argv[argc] = NULL;
    return execute_cmd(argv);
}
