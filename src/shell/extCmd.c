#include "extCmd.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_cmd(Command *cmd) {
    pid_t pid;
    int status;
    char **argv = cmd->args;

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

        /* Handle redirection, if necessary */

        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        /* mode gives read and write permissions to file owner,
         * and read permissions to the group owner and other users.
         */

        if (cmd->stdin_redirect != NULL) {
            int stdin_filedes = open(cmd->stdin_redirect->redirect_location,
                                     O_RDONLY); /* read only */
            dup2(stdin_filedes, STDIN_FILENO);
            close(stdin_filedes);
        }
        if (cmd->stdout_redirect != NULL) {
            /* write only; create file if it doesn't exist */  
            int stdout_filedes = open(cmd->stdout_redirect->redirect_location,
                                      O_WRONLY | O_CREAT | O_TRUNC, mode);
            
            dup2(stdout_filedes, STDOUT_FILENO);
            close(stdout_filedes);
        }
        if (cmd->stderr_redirect != NULL) {
            /* write only; create file if it doesn't exist */
            int stderr_filedes = open(cmd->stderr_redirect->redirect_location,
                                     O_WRONLY | O_CREAT, mode);
            
            dup2(stderr_filedes, STDERR_FILENO);
            close(stderr_filedes);
        }

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

int execute_ext_cmd(Command *cmd) {
    cmd->args[cmd->num_tokens] = NULL;
    return execute_cmd(cmd);
}
