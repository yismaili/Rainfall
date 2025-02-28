# Level0

After logging into the level0 account with the username `level0` and password `level0`, I found a binary file. Let's analyze it.

The `rws` permission indicates that the file is a setuid binary, meaning it runs with the privileges of its owner (`level1`).

```bash
level0@RainFall:~$ ls -l
total 732
-rwsr-x---+ 1 level1 users 747441 Mar  6  2016 level0
level0@RainFall:~$
```

When running this binary file, the output was:

```bash
level0@RainFall:~$ ./level0
Segmentation fault (core dumped)
level0@RainFall:~$ ./level0 e
No !
level0@RainFall:~$
```

## **Binary Analysis**

Using GDB to analyze the binary, we observe that the `main` function contains the following comparison:

```
0x08048ecf <+15>:  mov    (%eax),%eax
0x08048ed1 <+17>:  mov    %eax,(%esp)
0x08048ed4 <+20>:  call   0x8049710 <atoi>
0x08048ed9 <+25>:  cmp    $0x1a7,%eax
0x08048ede <+30>:  jne    0x8048f58 <main+152>
0x08048ee0 <+32>:  movl   $0x80c5348,(%esp)
0x08048ee7 <+39>:  call   0x8050bf0 <strdup>
0x08048eec <+44>:  mov    %eax,0x10(%esp)
```

This section of code compares the input argument (converted to an integer via `atoi`) with `0x1a7` (423 in decimal).

## Vulnerability

The program accepts a command-line argument, converts it to an integer using `atoi()`, and compares it to 423 (0x1a7). If the input matches 423, the program:

1. If the input matches (0x1a7)
2. Calls `getegid()` and `geteuid()` to retrieve the effective GID and UID.
3. Uses `setresgid()` and `setresuid()` for privilege escalation.
4. Executes a shell using `execv()`.

## Exploit

By providing `423` as an argument, we can exploit this vulnerability to obtain a privileged shell:

```bash
level0@RainFall:~$ ./level0 423
$ cat /home/user/level1/.pass
1fe8a524fa4bec01ca4ea2a869af2a02260d4a7d5fe7e7c24d8617e6dca12d3a
```

This demonstrates a basic privilege escalation vulnerability. By supplying the correct "magic number" (423), the program grants elevated access and spawns a shell, allowing us to read the password for the next level.

## Flag

```bash
1fe8a524fa4bec01ca4ea2a869af2a02260d4a7d5fe7e7c24d8617e6dca12d3a
```