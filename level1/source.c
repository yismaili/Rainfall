#include <stdio.h>
#include <stdlib.h>

FILE* global_file_pointer;

void run() {
    char* message = "Good job, keep going!";
    fwrite(message, 1, 0x13, global_file_pointer);
    
    system("/bin/sh");
}

int main() {
    char buffer[80];
    
    gets(buffer);
    
    return 0;
}