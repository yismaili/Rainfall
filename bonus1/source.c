#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int number;
    char buffer[40];  // esp+0x14
    
    number = atoi(argv[1]);
    
    if(number > 9)
        return 1;
        
    // copy argv[2] to buffer using number*4 as size
    memcpy(buffer, argv[2], number * 4);
    
    // check if numbr equals 0x574f4c46 ("FLOW" in hex)
    if(number == 0x574f4c46) {
        execl("/bin/sh", "/bin/sh", NULL);
    }
    
    return 0;
}