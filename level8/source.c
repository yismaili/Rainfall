
#include <stdio.h>

int *auth;
int *service;

int  main(int argc, const char **argv, const char **envp)
{
  char ptr;
  char arr2[2]; 
  char arr1[128]; 

  while ( 1 )
  {
    printf("%p, %p \n", (const void *)auth, (const void *)service);
    if ( !fgets(arr1, 128, stdin) )
      break;
    if ( !memcmp(arr1, "auth ", 5u) )
    {
      auth = malloc(4);
      *(int *)auth = 0;
      if ( strlen(arr2) <= 30 )
        strcpy(auth, arr2);
    }
    if ( !memcmp(arr1, "reset", 5) )
      free(auth);
    if ( !memcmp(arr1, "service", 6) )
      service = strdup(&ptr);
    if ( !memcmp(arr1, "login", 5) )
    {
        if ( *(int *)(auth + 32) )
        system("/bin/sh");
        else
        fwrite("Password:\n", 1, 10, stdout);
    }
  }
  return 0;
}