#include "../video.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char icon = 'A';
    char color = BLACK;
    char output[9];
    itoa(icon, output, 2);
    printf("%s\n", output);
    return 0;
}
