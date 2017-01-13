#include "extCmd.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_cmd(Command *cmd, CommandLinkedList *cmds, int input_fd) {
    pid_t pid;
    int status;
    int pipefd[2];
    char **argv = cmd->args;
    bool has_pipe = (cmd->next_command != NULL);

    /* If there is a pipe to a next command, create a new pipe */
    if (has_pipe) {
        if (pipe(pipefd) < 0) {
            fprintf(stderr, "Pipe failed\n");
            exit(1);
        }
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
            /* Check that stdin redirect file is valid */
            if (access(cmd->stdin_redirect->redirect_location, F_OK) == -1) {
                fprintf(stderr, "no such file: %s\n",
                        cmd->stdin_redirect->redirect_location);
                CommandLinkedList_free_pointer(cmds);
                exit(0);
            }

            int stdin_filedes = open(cmd->stdin_redirect->redirect_location,
                    O_RDONLY); /* read only */
            dup2(stdin_filedes, STDIN_FILENO);
            close(stdin_filedes);
        }
        if (cmd->stdout_redirect != NULL) {
            /* Check that output file location is not existing directory */
            DIR* dir = opendir(cmd->stdout_redirect->redirect_location);
            if (dir) {
                fprintf(stderr, "is a directory: %s\n",
                        cmd->stdout_redirect->redirect_location);
                CommandLinkedList_free_pointer(cmds);
                free(dir);
                exit(0);
            }

            /* write only; create file if it doesn't exist */
            int stdout_filedes;
            if (cmd->stdout_redirect->redirect_type == OUT_REDIR) {
                stdout_filedes = open(cmd->stdout_redirect->redirect_location,
                                      O_WRONLY | O_CREAT | O_TRUNC, mode);
            }
            else {
                /* Append instead of truncate */
                stdout_filedes = open(cmd->stdout_redirect->redirect_location,
                                      O_RDWR | O_CREAT | O_APPEND, mode);
            }

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
            close(pipefd[0]);
            if (dup2(input_fd, STDIN_FILENO) < 0) {
                fprintf(stderr, "Error redirecting stdin\n");
            }
            if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
                fprintf(stderr, "Error redirecting stdout\n");
            }
            else if (close(pipefd[1]) < 0) {
                fprintf(stderr, "Error closing extra pipe fd\n");
            }
            else {
                /* This process should never return if successful */
                execvp(argv[0], argv);
                fprintf(stderr, "execvp failed - unknown command %s\n",
                        argv[0]);
                CommandLinkedList_free_pointer(cmds);
                exit(0);
            }
        }
        /* If there is no pipe, execute the single command */
        else {
            if (input_fd != STDIN_FILENO) {
                if (dup2(input_fd, STDIN_FILENO) != -1) {
                    close(input_fd);
                }
                else {
                    fprintf(stderr, "Error redirecting stdin\n");
                }
            }

            /* This process should never return if successful */
            execvp(argv[0], argv);
            fprintf(stderr, "execvp failed - unknown command %s\n", argv[0]);
            CommandLinkedList_free_pointer(cmds);
            exit(0);
        }
    }

    /* Parent process */
    else {
        /* Wait for child process to terminate */
        pid_t finished_pid = wait(&status);
        while (finished_pid != pid) {
            finished_pid = wait(&status);
        }

        if (has_pipe) {
            /* If there is a pipe, replace stdout with pipe output */
            close(pipefd[1]);
            close(input_fd);
            /* If there is a pipe, execute the next command. */
            execute_cmd(cmd->next_command, cmds, pipefd[0]);
        }

        return status;
    }
    return -1; /* should not get here */
}

int execute_ext_cmd(Command *cmd, CommandLinkedList *cmds) {
    int save_in, save_out, res;
    save_in = dup(STDIN_FILENO);
    save_out = dup(STDOUT_FILENO);

    /* Initial stdin should be STDIN_FILENO */
    res = execute_cmd(cmd, cmds, STDIN_FILENO);

    /* Restore stdin and stdout */
    dup2(save_in, STDIN_FILENO);
    dup2(save_out, STDIN_FILENO);

    return res;
}
