#include "tokenizer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

token_type *tokenize(char *line)
{
  size_t len = strlen(line);
  static token_type *tokens;
  tokens = malloc(len * sizeof(token_type));
  bool in_string = false;

  for (int i = 0; i < len; i++)
  {
    char curr_char = line[i];
    token_type curr_token = get_token(in_string, curr_char);
    if (!in_string)
    {
      if (curr_token == QUOTE)
      {
        in_string = true;
      }
      tokens[i] = curr_token;
    }
    else {
      if (curr_token == QUOTE)
      {
        in_string = false;
      }
      // Parse two characters when handling backslash
      else if (curr_char == '\\')
      {
        tokens[i] = curr_token;
        i++;
        curr_char = line[i];
        curr_token = CHAR;
      }
      tokens[i] = curr_token;
    }
  }

  return tokens;
}

token_type get_token(bool in_string, char curr_char)
{
  if (curr_char == '\"')
  {
    return QUOTE;
  }
  // If inside quotes, must be character
  else if (in_string)
  {
    return CHAR;
  }
  else if (curr_char == '|') {
    return PIPE;
  }
  else if (curr_char == '<' || curr_char == '>')
  {
    return REDIRECT;
  }
  else if (curr_char == ' ' || curr_char == '\t')
  {
    return WHITE;
  }
  else
  {
    return CHAR;
  }
}
