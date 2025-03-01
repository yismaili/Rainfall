#include <stdio.h>

int m = 0;

void v(void)

{
  char input [520];
  
  fgets(input,0x200,stdin);
  printf(input);
  if (m == 0x40) {
    fwrite("Wait what?!\n",1,0xc,stdout);
    system("/bin/sh");
  }
  return;
}



int main() {
    v();
    return 1 ;
}