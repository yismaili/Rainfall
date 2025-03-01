#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    FILE *file;
    char buffer[132];  // esp+0x18
    
    file = fopen("/home/user/level4/.pass", "r");
    
    memset(buffer, 0, 132);
    
    if(!file || argc != 2)
        return -1;
        
    // read first 66 bytes to buffer
    fread(buffer, 1, 66, file);
    buffer[65] = '\0';
    
    // get number from argv[1] and null terminate at that position
    int pos = atoi(argv[1]);
    buffer[pos] = '\0';
    
    // read next 65 bytes to buffer+66
    fread(buffer + 66, 1, 65, file);
    
    fclose(file);
    
    // compare input with first part of buffer
    if(strcmp(buffer, argv[1]) == 0) {
        execl("/bin/sh", "sh", NULL);
    }
    else {
        puts(buffer + 66);
    }
    
    return 0;
}