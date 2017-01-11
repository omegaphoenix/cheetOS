#include "tokenizer.h"
#include "commandStruct.h"

#include <stdio.h>

int main() {
  // char str[KiB(1)];

  // printf("Enter a string:");
  // fgets(str, KiB(1), stdin);

  // printf("\nYou entered: %s ", str);
  // printf("\n");
  int idx;
  int num_words = 8;
  char *input[] = {"a", "arg1", "arg2", "<", "logout.txt", ">", "weiner.txt", "poop"};
  token_type tokens[] = {WORD, WORD, WORD, IN_REDIR, WORD, OUT_REDIR, WORD, WORD};

  Command *new_command = Command_new_pointer(input, tokens, num_words);
  if (new_command) {
    printf("new_command size: %d\n", new_command->num_tokens);

    for (idx = 0; idx < new_command->num_tokens + 1; idx++) {
      printf("command: %s\n", new_command->args[idx]);
    }

    if (new_command->stdin_redirect->redirect_type == IN_REDIR) {
      printf("Redirect location for stdin: %s\n", new_command->stdin_redirect->redirect_location);
    }

    if (new_command->stdout_redirect->redirect_type == OUT_REDIR) {
      printf("Redirect location for stdout: %s\n", new_command->stdout_redirect->redirect_location);
    }
  
    Command_free_pointer(new_command);
  }

  return 0;
}