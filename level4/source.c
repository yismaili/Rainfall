#include <stdio.h>

int m = 0;

void n(void)

{
    char input [520];

    fgets(input,0x200,stdin);
    p(input);
    if (m == 0x1025544) {
    system("/bin/cat /home/user/level5/.pass");
    }
    return;
}

int main() {
    n();
    return 1;
}