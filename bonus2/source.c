#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int language = 0; // at 0x8049988

void greetuser() {
    char greeting[72];  // ebp-0x48

    switch(language) {
        case 0:
            strcpy(greeting, "Hello ");  // 7 bytes
            break;
        case 1:
            strcpy(greeting, "Hyvää päivää ");  // 19 bytes
            break;
        case 2:
            strcpy(greeting, "Goedemiddag! ");  // 14 bytes
            break;
    }
    
    // concatenate user input and print
    strcat(greeting, (char*)ebp+8); 
    puts(greeting);
}

int main(int argc, char *argv[]) {
    char buffer[76];  // esp+0x50
    char *lang;
    
    if(argc != 3)
        return 1;
        
    memset(buffer, 0, 76);
    
    // copy first arg (max 40 bytes)
    strncpy(buffer, argv[1], 40);
    
    // copy second arg (max 32 bytes) at offset 40
    strncpy(buffer + 40, argv[2], 32);
    
    // check LANG environment variable
    lang = getenv("LANG");
    if(lang != NULL) {
        if(memcmp(lang, "fi", 2) == 0) {
            language = 1;
        }
        else if(memcmp(lang, "nl", 2) == 0) {
            language = 2;
        }
    }
    
    // Copy buffer to stack and call greetuser
    char stackbuf[76];
    memcpy(stackbuf, buffer, 76);
    greetuser();
    
    return 0;
}