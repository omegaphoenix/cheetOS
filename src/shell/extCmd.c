#include "extCmd.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_cmd(char **argv) {
    pid_t pid;
    int status;

    /*
     * Fork off a child process to execute program
     * Fork will create two processes; child process will
     * have pid of 0, and parent will have a pid > 0
     */
    if ((pid = fork()) < 0) {
        fprintf(stderr, "Fork failed\n");
        exit(1);
    }

    /* Child process */
    if (pid == 0) {
        /* Execute command with child process */

        /* This process should never return if successful */
        execvp(argv[0], argv);
        fprintf(stderr, "execvp failed - unknown command\n");
        exit(0);
    }

    /* Parent process */
    else {
        /* Wait for child process to terminate */
        pid_t finished_pid = wait(&status);
        while (finished_pid != pid) {
            finished_pid = wait(&status);
        }

        return status;
    }
}

int execute_ext_cmd(int argc, char **argv) {
    argv[argc] = NULL;
    return execute_cmd(argv);
}
