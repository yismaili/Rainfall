
#include <stdio.h>
#include <unistd.h>


int main(int argc,char **argv)

{
    int nbr;
    char *ptr;
    __uid_t uid;
    __gid_t gid;

    nbr = atoi(*(argv[1]));
    if (nbr == 423) {
        ptr = strdup("/bin/sh");
        gid = getegid();
        uid = geteuid();
        setresgid(gid,gid,gid);
        setresuid(uid,uid,uid);
        execv("/bin/sh",&ptr);
    }
    else {
        fwrite("No !\n",1,5,stderr);
    }
    return 0;
}

