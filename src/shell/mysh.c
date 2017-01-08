#include "tokenizer.h"

#include <stdio.h>

int main()
{
  char str[100];

  // Use fgets()
  printf("Enter a string:");
  scanf("%s", str);

  printf("\nYou entered: %s ", str);
  printf("\n");
  return 0;
}
