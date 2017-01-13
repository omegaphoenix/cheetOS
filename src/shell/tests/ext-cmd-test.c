#include "../extCmd.h"
#include "../commandStruct.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Test grep */
void executeCmd_externalCommand_shouldGrep() {
    char *cmd = "grep";
    char *search_word = "all";
    char *file = "Makefile";

    char *words[] = {cmd, search_word, file};
    token_type tokens[] = {WORD, WORD, WORD};
    int num_tokens = 3;

    /* Create command struct */
    Command *test_cmd = malloc(sizeof(Command));
    test_cmd = Command_new_pointer(words, tokens, num_tokens);

    /* Expected behavior */
    printf("Testing external command \"%s\"\n", cmd);
    printf("Should output line with the word \"%s\"\n", search_word);

    execute_cmd(test_cmd, STDIN_FILENO);
}

int main() {
    executeCmd_externalCommand_shouldGrep();
    return 0;
}
