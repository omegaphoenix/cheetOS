#include "../extCmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define KiB(x) ((size_t) (x) << 10)
#define PATH_MAX KiB(1)

/* Test cd with no arguments */
void executeCmd_externalCommand_shouldGrep() {
    char *cmd = "grep";
    char *search_word = "all";
    char *command[] = {cmd, search_word, "Makefile", NULL};

    printf("Testing external command \"%s\"\n", cmd);
    printf("Should output line with the word \"%s\"\n", search_word);
    execute_cmd(command);
}

int main() {
    executeCmd_externalCommand_shouldGrep();
    return 0;
}
