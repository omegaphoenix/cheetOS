#include "../builtin.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define KiB(x) ((size_t) (x) << 10)
#define PATH_MAX KiB(1)

/* Test cd with no arguments */
void cd_noArg_home() {
    char *cwd;
    char cwd_buffer[PATH_MAX + 1];
    char *args[] = {"cd", NULL};

    printf("TEST cd WITH NO ARGS\n");

    /* Print current working directory */
    cwd = getcwd(cwd_buffer, PATH_MAX + 1);
    if (cwd != NULL) {
        printf("Current directory: %s\n", cwd);
    }

    /* Run command: cd */
    in_cd(args);

    /* Print results */
    cwd = getcwd(cwd_buffer, PATH_MAX + 1);

    if (cwd == NULL) {
        fprintf(stderr, "Error: cwd returned NULL\n");
    }
    else if (strcmp(cwd, getenv("HOME")) != 0) {
        fprintf(stderr, "Expected directory after cd: %s\n", getenv("HOME"));
        fprintf(stderr, "Actual directory: %s\n", cwd);
    }
    else {
        printf("Test passes!\n");
    }

    printf("\n");
}

/* Test cd with single and multiple arguments */
void cd_singleArg_specifiedDirectory() {
    char *cwd;
    char cwd_buffer[PATH_MAX + 1];
    char *args[] = {"cd", "..", NULL};
    char *args2[] = {"cd", "..", "thisArgShouldDoNothing", NULL};

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

void exit_standard_exit() {
    char *args[] = {"exit", NULL};
    in_exit(args);
}

int main() {
    cd_noArg_home();
    cd_singleArg_specifiedDirectory();

    /* Test exit */
    printf("Now, the test program should exit.\n");
    exit_standard_exit();
    printf("If you see this line, in_exit() failed.\n");
    return 0;
}
