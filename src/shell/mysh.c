#include <stdio.h>
#include "tokenizer.c"
int main()
{
  char str[100];

  printf("Enter a string:");
  scanf("%s", str);

  printf("\nYou entered: %s ", str);
  printf("\n");
  return 0;
}
