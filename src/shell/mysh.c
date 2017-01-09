#include "tokenizer.h"

#include <stdio.h>

int main() {
  char str[KiB(1)];

  printf("Enter a string:");
  fgets(str, KiB(1), stdin);

  printf("\nYou entered: %s ", str);
  printf("\n");
  return 0;
}
