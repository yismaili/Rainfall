#include <stdio.h>
#include <stdlib.h>

void p(char *buffer) {
    printf(buffer);
}

void n() {
    char buffer[520];
    fgets(buffer, 512, stdin);
    p(buffer);
    
    if (*(int*)0x8049810 == 0x1025544) {
        system("/bin/sh");
    }
}

int main() {
    n();
    return 0;
}