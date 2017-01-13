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
    int pipefd[2];
    char **argv = cmd->args;
    bool has_pipe = (cmd->next_command != NULL);

    /* If there is a pipe to a next command, create a new pipe */
    if (has_pipe) {
        fprintf(stderr, "pipe detected\n");
        pipe(pipefd);
    }
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

        /* If there is a pipe, replace stdin with pipe input */
        if (has_pipe) {
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[1]);

            /* Execute the command after the pipe */
            fprintf(stderr, "trying to execute next cmd\n");
            execute_cmd(cmd->next_command);
        }
        /* If there is no pipe, execute the single command */
        else {
            /* This process should never return if successful */
            fprintf(stderr, "executing single cmd %s\n", argv[0]);
            execvp(argv[0], argv);
            fprintf(stderr, "execvp failed - unknown command\n");
            exit(0);
        }
    }

    /* Parent process */
    else {
        /* If there is a pipe, replace stdout with pipe output */
        if (has_pipe) {
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
        }

        /* Wait for child process to terminate */
        pid_t finished_pid = wait(&status);
        while (finished_pid != pid) {
            finished_pid = wait(&status);
        }

        /* If there is a pipe, execute the command.
         * (if there is no pipe, the child executed the command already) */
        if (has_pipe) {
            /* This process should never return if successful */
            execvp(argv[0], argv);
            fprintf(stderr, "execvp failed - unknown command\n");
            exit(0);
        }

        return status;
    }
    return 1; /* should not get here */
}

int execute_ext_cmd(Command *cmd) {
    cmd->args[cmd->num_tokens] = NULL;
    return execute_cmd(cmd);
}
