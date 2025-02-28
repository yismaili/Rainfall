int main(int argc, char *argv[]) 
{
    if (atoi(argv[1]) == 423) {
        char *duplicated_string = strdup(0x80c5348);
        
        gid_t egid = getegid();
        uid_t euid = geteuid();
        
        setresgid(egid, egid, egid);
        
        setresuid(euid, euid, euid);
        
        execv(0x80c5348, &duplicated_string);
    } else {
        fwrite(error_message, 1, 5, stderr);
    }
    
    return 0;
}