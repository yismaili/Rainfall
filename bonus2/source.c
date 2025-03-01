#include <stdio.h>

char fi_glob[] = "fi";

char nl_glob[] = "nl";

int language = 0;


int greetuser(char *user)
{
      char greet[68];
  
       switch(language) {
        case 0:
            strcpy(greet, "Hello ");  // 7 bytes
            break;
        case 1:
            strcpy(greet, "Hyvää päivää ");  // 19 bytes
            break;
        case 2:
            strcpy(greet, "Goedemiddag! ");  // 14 bytes
            break;
    }

    strcat(greet,user);
    puts(greet);
    return 1;
}



int main(int argc,char **argv)

{
    int ret;
    int i;
    char arr2 [0x20];
    char arr1 [0x28];
    char *env_ptr;
    char *ptr;


    if (argc == 3) {
    ptr = arr1;
    for (i = 0x13; i != 0; i = i + -1) {
        *ptr = 0;
        ptr = ptr + 1;
    }
    strncpy(arr1,argv[1],0x28);
    strncpy(arr2,argv[2],0x20);
    env_ptr = getenv("LANG");
    if (env_ptr != 0x0) {
        i = memcmp(env_ptr,&fi_glob,2);
        if (i == 0) {
        language = 1;
        }
        else {
        i = memcmp(env_ptr,&nl_glob,2);
        if (i == 0) {
            language = 2;
        }
        }
    }
    ret = greetuser(arr1);
    }
    else {
    ret = 1;
    }
    return ret;
}

