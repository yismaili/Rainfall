#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void p() {
    char buffer[64];
    unsigned int ret_addr;

    fflush(stdout);

    gets(buffer);

    ret_addr = *((unsigned int*)($ebp + 4));

    if ((ret_addr & 0xb0000000) == 0xb0000000) {
        printf("%x\n", ret_addr);
        _exit(1);
    }

    puts(buffer);
    strdup(buffer);
}

int main() {
    p();
    return 0;
}