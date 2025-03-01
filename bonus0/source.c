

#include <stdio.h>


int  p(char *array, char *array2)
{
    char arr_p[4104];

    puts(array2);
    read(0, arr_p, 4096);
    *(char *)strchr(arr_p, 10) = 0;
    return strncpy(array, arr_p, 20);
}


int  pp(char *a1)
{
    char a2[20];
    char a3[28];

    p(a2, "-");
    p(a3, "-");
    strcpy(a1, a2);
    a1[strlen(a1)] = ' ';
    return strcat(a1, a3);
}



int  main(int argc, const char **argv, const char **envp)
{
    char array[42];

    pp(array);
    puts(array);
    return 0;
}