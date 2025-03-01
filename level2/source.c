#include <stdio.h>


void p(void)

{
    unsigned int addr;
    char input [76];

    fflush(stdout);
    gets(input);
    if ((addr & 0xb0000000) == 0xb0000000) {
    printf("(%p)\n",addr);
    _exit(1);
    }
    puts(input);
    strdup(input);
    return;
}



int main()  { 
    p();
    return;
}