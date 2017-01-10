#include "../builtin.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PATH_MAX 80

/* Test cd with no arguments */
void cd_noArg() {
    char *cwd;
    char cwd_buffer[PATH_MAX + 1];
    char **args = (char *[]){"cd", NULL};

    printf("TEST cd WITH NO ARGS\n");

    /* Print current working directory */
    cwd = getcwd(cwd_buffer, PATH_MAX + 1);
    if (cwd != NULL) {
        printf("Current directory: %s\n", cwd);
    }

    /* Run command: cd */
    in_cd(args);

    /* Print results */
    printf("Expected directory after cd: %s\n", getenv("HOME"));
    cwd = getcwd(cwd_buffer, PATH_MAX + 1);
    if (cwd != NULL) {
        printf("Got: %s\n", cwd);
    }

    printf("\n");
}

/* Test cd with single and multiple arguments */
void cd_singleArg() {
    char *cwd;
    char cwd_buffer[PATH_MAX + 1];
    char **args = (char *[]){"cd", "..", NULL};
    char **args2 = (char *[]){"cd", "..", "thisArgShouldDoNothing", NULL};

    printf("TEST cd WITH ONE ARG\n");

    /* Print current working directory */
    cwd = getcwd(cwd_buffer, PATH_MAX + 1);
    if (cwd != NULL) {
      printf("Current directory: %s\n", cwd);
    }

    /* Run command: cd .. */
    in_cd(args);

    printf("Expected directory after cd: one directory up from %s\n", cwd);

    /* Print results */
    cwd = getcwd(cwd_buffer, PATH_MAX + 1);
    if (cwd != NULL) {
        printf("Got: %s\n", cwd);
    }

    printf("\n");
    printf("TEST cd WITH TWO ARGS\n");

    /* Print current directory */
    cwd = getcwd(cwd_buffer, PATH_MAX + 1);
    if (cwd != NULL) {
        printf("Current directory: %s\n", cwd);
    }

    /* Run command: cd .. thisArgShouldDoNothing */
    in_cd(args2);

    /* Print results */
    printf("Expected directory after cd: one directory up from %s\n", cwd);
    cwd = getcwd(cwd_buffer, PATH_MAX + 1);
    if (cwd != NULL) {
        printf("Got: %s\n", cwd);
    }

    printf("\n");
}

void exit_standard() {
    char **args = (char *[]){"exit", NULL};
    in_exit(args);
}

int main() {
    cd_noArg();
    cd_singleArg();

    /* Test exit */
    printf("Now, the test program should exit.\n");
    exit_standard();
    printf("If you see this line, in_exit() failed.\n");
    return 0;
}
