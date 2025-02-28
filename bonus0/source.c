#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void p(char *buffer, char *str) {
    char tmp[4104];  // 0x1008 bytes
    
    puts(str);
    read(0, tmp, 4096);  // read up to 4096 bytes
    
    
    char *newline = strchr(tmp, '\n');
    if(newline) *newline = 0;
    
    // 20 bytes to buffer
    strncpy(buffer, tmp, 20);
}

void pp(char *buffer) {
    char buf1[20];  // -0x30(%ebp)
    char buf2[20];  // -0x1c(%ebp)
    
    p(buf1, " - ");  
    p(buf2, " - ");
    
    strcpy(buffer, buf1);
    
    char *end = buffer + strlen(buffer);
    *(short*)end = 0x2020;  // Two spaces
    
    // Concatenate buf2
    strcat(buffer, buf2);
}

int main(int argc, char *argv[]) {
    char buffer[100];
    pp(buffer);
    return 0;
}