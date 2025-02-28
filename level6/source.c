#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void n() {
    system("/bin/cat /home/user/level7/.pass");
}

void m() {
    puts("Nope");
}

int main(int argc, char *argv[]) {
    char *buffer1;
    char **buffer2;

    buffer1 = malloc(64);
    buffer2 = malloc(4);
    
    *buffer2 = (char*)m;
    
    strcpy(buffer1, argv[1]);
    
    ((void(*)())(*buffer2))();
    
    return 0;
}