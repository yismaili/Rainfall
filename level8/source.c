#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct auth {
    char *username;
    int auth;
};

int main() {
    char line[128];
    struct auth *auth;
    
    while(1) {
        printf("%p, %p \n", auth, auth->username);
        
        if(fgets(line, 128, stdin) == NULL)
            break;
            
        if(strncmp(line, "auth ", 5) == 0) {
            auth = malloc(sizeof(struct auth));
            auth->auth = 0;
            if(strlen(line + 5) < 30)
                strcpy(auth->username, line + 5);
        }
        
        if(strncmp(line, "reset", 5) == 0) {
            free(auth);
        }
        
        if(strncmp(line, "service", 6) == 0) {
            auth->username = strdup(line + 7);
        }
        
        if(strncmp(line, "login", 5) == 0) {
            if(auth->auth == 32) {
                system("/bin/sh");
            } else {
                fwrite("Password:\n", 1, 10, stdout);
            }
        }
    }
    return 0;
}