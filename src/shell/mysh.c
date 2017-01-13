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

void free_char_array(char **char_array, int size_of_array) {
    int idx;
    if (char_array) {
        for (idx = 0; idx < size_of_array; idx++) {
            free(char_array[idx]);
        }
        free(char_array);
    }
}

int main() {
    char *cwd = NULL;
    char *username = NULL;
    char cwd_buffer[MAX_INPUT + 1];

    char input_str[MAX_INPUT];

    char **words = NULL;
    token_type *tokens = NULL;
    CommandLinkedList *commands = NULL;
    Command *curr_command = NULL;

    TokenGroupLList *word_groups = NULL;
    TokenGroup *curr_group = NULL;

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

        /* Malloc checker */
        if (!words || !tokens) {
          goto free_malloc;
        }

        num_tokens = parse_tokens(input_str, words, tokens);
        word_groups = split_string_by_pipe(words, tokens, num_tokens);

        /* Creating command linked list delimited by pipes */
        if (word_groups) {
            commands = CommandLinkedList_new_pointer();

            curr_group = word_groups->first_group;
            while (curr_group) {
                curr_command = Command_new_pointer(curr_group->words,
                                                   curr_group->tokens,
                                                   curr_group->num_tokens);

                if (!curr_command || !commands) {
                    goto free_malloc;
                }

                command_linked_list_append(commands, curr_command);
                curr_group = curr_group->next_group;
            }
        };

        /* Skip if no input */
        if (num_tokens == 0) {
        }
        /* TODO: improve this by looping through array of internal cmds */
        /* If internal command, execute in current process */
        else if (strcmp(words[0], "cd") == 0) {
            in_cd(words);
        }
        else if (strcmp(words[0], "exit") == 0) {
            free_char_array(words, num_tokens);
            free(tokens);
            CommandLinkedList_free_pointer(commands);
            TokenGroupLList_free(word_groups);
            in_exit(words);
        }

        else {
            /* Passing everything in to potentially garbage collect */
            execute_ext_cmd(commands->first_command);
        }

        free_malloc:
          free_char_array(words, num_tokens);
          free(tokens);
          CommandLinkedList_free_pointer(commands);
          TokenGroupLList_free(word_groups);
    }
    return 0;
}
