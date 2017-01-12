#include "builtin.h"
#include "commandStruct.h"
#include "extCmd.h"
#include "tokenizer.h"
#include "tokenStruct.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT KiB(1)

int main() {
    char *cwd = NULL;
    char *username = NULL;
    char cwd_buffer[MAX_INPUT + 1];

    char input_str[MAX_INPUT];

    char **words = NULL;
    token_type *tokens = NULL;
    int num_tokens;

    while (1) {
        /* Prompt with username and current working directory */
        cwd = getcwd(cwd_buffer, MAX_INPUT + 1);
        username = getlogin();

        printf("%s:%s> ", username, cwd);
        fgets(input_str, MAX_INPUT, stdin);

        /* Remove newline (and any following chars) at the end of input_str */
        input_str[strcspn(input_str, "\n")] = 0;

        /* Tokenize input command */
        words = malloc(MAX_INPUT);
        tokens = malloc(MAX_INPUT * sizeof(token_type));
        num_tokens = parse_tokens(input_str, words, tokens);

        /* TODO: Create Command struct from tokens */
        if (num_tokens) {}; /* delete this */

        /* TODO: improve this by looping through array of internal cmds */
        /* If internal command, execute in current process */
        if (strcmp(words[0], "cd") == 0) {
            in_cd(words);
        }
        else if (strcmp(words[0], "exit") == 0) {
            free(words);
            free(tokens);
            in_exit(words);
        }

        /* TODO: Else, fork a child process and execute */
        else {
            /* TODO: Swap wrapper function once command is been used */
            execute_ext_cmd(num_tokens, words);
        }

        free(words);
        free(tokens);
    }
    return 0;
}
