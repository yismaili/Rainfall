#include <stdio.h>
#include <stdlib.h>

void v() {
    char buffer[520];
    fgets(buffer, 512, stdin);
    printf(buffer);
    
    if (*(int*)0x804988c == 0x40) {
        fwrite("You win!\n", 1, 12, stdout);
        system("/bin/sh");
    }
}

int main() {
    v();
    return 0;
}