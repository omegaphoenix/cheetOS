#include "builtin.h"
#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    char *cwd;
    char *username;
    char cwd_buffer[KiB(1) + 1];

    char input_str[KiB(1)];

    while (1) {
        /* Prompt with username and current working directory */
        cwd = getcwd(cwd_buffer, KiB(1) + 1);
        username = getlogin();

        printf("%s:%s> ", username, cwd);
        fgets(input_str, KiB(1), stdin);

        /* Remove newline (and any following chars) at the end of input_str */
        input_str[strcspn(input_str, "\n")] = 0;

        /* Tokenize input command */
        char **words = malloc(KiB(1));
        token_type *tokens = malloc(KiB(1) * sizeof(token_type));
        int num_tokens = parse_tokens(input_str, words, tokens);

        /* TODO: Create Command struct from tokens */
        if (num_tokens) {}; /* delete this */

        /* TODO: improve this by looping through array of internal cmds */
        /* If internal command, execute in current process */
        if (strcmp(words[0], "cd") == 0) {
            in_cd(words);
        }
        else if (strcmp(words[0], "exit") == 0) {
            in_exit(words);
        }

        /* TODO: Else, fork a child process and execute */
        else {
            printf("You entered: %s", input_str);
            printf("\n");
        }
    }
    return 0;
}
