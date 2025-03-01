#include <stdio.h>
#include <stdlib.h>

FILE* global_file_pointer;

void run() {
    char* message = "Good... Wait what?";
    fwrite(message, 1, 0x13, global_file_pointer);
    
    system("/bin/sh");
}

int main() {
    char buffer[72];
    
    gets(buffer);
    
    return 0;
}