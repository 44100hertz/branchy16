# Planned operations

Add these to CPU:
 - Binary bit operations AND OR XOR
 - Binary add, sub, cadd, csub, Add/Subtract with and without carry
 - Set and clear carry (see below)
 - Conditional jump on carry (thru extra bit in JUMP instruction)
 - Shift left or right by 1 bit, with or without carry in
 - Shift right with sign extension
 - Signed compare (add flag to compare)

Remember: instructions should be unique to one another in the first few characters, so that they can easily be autocompleted.
```
ITAG_SETFLAG: xxxxx FFF AAAA --oo
FFF: flag index
AAAA: argument nibble. Set flag to value != 0

REPLACES HALT!

CPU flags:
0 - running
1 - carry
2 - zero
3 - neg
4 - eq
5 - gt
6 - lt
```
# New Flags
 - Zero flag which is set any time the destination is zero
   - For byte or nibble ops, set zero accordingly
 - Negative flag which is set any time destination high bit set
   - For byte or nibble ops, set negative accordingly
 - Conditional jump based on those flags

# Potential operations

## Byte ops
 - unary Sign extend lower byte, overwriting upper
 - unary Shift and sign extend upper byte
 - unary Copy upper or lower byte between registers
   - Allow self copying to act much like 8-bit shift

## Nibble ops (needs more design work)
 - Copy any given nibble
 - Shift nibbles left or right

## Bit Ops
 - Set bit of register
 - Test bit of register, set "eq" if 0 and "gt" if 1
```
ITAG_SETBIT: xxxxx FFF cDDD BBBB
FFF: flag index. index 7 = 0, index 0 = 1*
DDD: destination register
c: clear register to 0 before setting flag
BBBB: bit index in register to set
* index 0 is 1 because 0 is the "running" flag which must be 1 lmao

ITAG_TESTBIT: xxxxx --- -SSS BBBB
DDD: source register
BBBB: bit index to test
```

## Mutex operation
Currently a mutex lock looks like this:
```
  load  r0  mutex
test_mutex:
  cmpr  r0  0
  jumpe free
  loadw r0  mutex   ;; wait for mutex value
  jump  test_mutex
free:
  store mutex  1    ;; lock for the next thread
  load  r0  addr    ;; do anything with the locked value
```
One problem off the bat is that storing a 1 to the mutex can't be done atomically, so there's a possibility of another thread loading the "0" causing the mutex to fail.

Putting all of this code (except the load at the end) into a single atomic instruction seems like the best approach.

```
ITAG_SYNC xxxxx c-- AAAA ----

Waits for address A to equal zero, then resumes execution. A is then set to 0 if c is clear, and 1 if set.
```

## Load-Wait non-exclusive

In order to do a non-exclusive load-wait, we have to do this on every branch:
```
loadw r0  addr
store r0  addr
...
```
This allows the next branch to be freed, and the next, etc. which is slow. Meanwhile, for I/O instructions, everything unlocks at the same time. If the above mutex instruction is implemented, then load-waits can become no longer exclusive.

## Misc ops
 - Copy all CPU flags to and from register
 - Copy base pointer or PC to and from registers

# Stack

Right now branchy16 has no stack, which makes subroutines not really possible. Adding a stack pointer into RAM to every branch kind of stinks, but maybe each branch can have its own dedicated stack in some hidden RAM. This also means adding a stack pointer register, as well as push, pop, jump to subroutine, and return instructions.

# Replace relative addressing with stack addressing

Since each branch will have a stack, maybe instead of the "base pointer" 

# Get rid of "branch" name

Branches should probably be named "threads". "Branch" is misleading. Maybe "limb".

# Making PC and BP accessible in register nibble

Right now there are 16 register indexes possible. 8 of them are GPRs, and 3 of them are constant values, giving 5 unused slots. The PC and BP could be put into these slots, though this also opens up the silly functionality of, for example, XOR'ing the base pointer.

# Multiplication etc.

Currently, multiplication, division, and modulo is not planned to be supported. Write a multiplication loop, or find one :)

# extended ROM device
120k is only enough for a demo.

Each full frame of video is 19KiB. I estimate that 100 full frames (1.9MiB) is enough for a full game.

Each second of high quality audio is 64KiB. I estimate that 20 seconds of sampled audio (1.2MiB) is enough for a soundtrack with a few drum loops.

So how big? 1.9MiB + 1.2MiB = roughly 3MiB, or 24 megabits, which is 4 times the 6 megabits of Kirby's adventure (which used compression). The largest SNES games were 48 megabits, or 6MiB, and the largest GBA games were 32MiB.

Bank switching can be a headache, but we have enough RAM to copy from the ROM instead. Since video is so small, sharing it with the main RAM is sensible enough, allowing code and video data to be loaded into RAM as needed. "Bank switching" graphical effects can be achieved fairly easily with RAM write loops or by changing pointers around.

The audio device should be have direct ROM device access for sample playback.

# Save Data
What's a game without save data? Password systems are antiquated and annoying. If game servers use accounts, then they should also store save data.

How big? Different genres warrant different save data sizes. For an RPG with 4000 collectibles, that's 4000 bits or 250 words. For a game with a 1000x1000 generated world one byte per tile, that's 1MB.

How fast? Saving should be slow enough that save data isn't used as a secondary RAM device. 600 word per second would allow one screen of a generated world to be loaded in 1 second, but then, reading all 1MB of that data would take 13 minutes. To make it take 60 seconds, 16.7k words per second are needed, which nears the speed of existing RAM.

# Networking / Multiplayer
Something like a networked "game link cable" that can go over the network is warranted to enhance the social aspect of games. For game servers...no idea.

I won't design it all here, but it's a consideration.

# Screen and blanking

Right now the screen resolution is 240x160, or 15x10 tiles. I think the ideal resolution is actually 288x192, or 18x12 tiles. This preserves the 3:2 aspect originally intended, while also offering a decent 240p letterboxed solution for CRT displays. The internal horizontal resolution would be 640px, but 576px would be displayed, keeping the game in the safe region of 568px, minus 4px on each side which _could_ be overscanned but likely will be tolerable.

This also changes the clock speed and blanking calculations.

NTSC scanning region is 525 lines, giving 384 lines of draw and 141 lines of blanking. 

NTSC active line period is defined as 52.9µs, and my calculations figure that 63.5µs are needed per scanline for 525 lines at 60hz. For a 320px active line, this gives a 384px scan region, out of which 288px is used, giving 96px of hblank.

So overall, the scanning region is 384x262.5 at 60hz, or 6.048mhz. With 8 cycles per scanline, this gives 6 cycles of draw and 2 cycles of blanking, and a final clock speed of 8 * 262.5 * 60 = 126khz per branch.

# Fetching versus random access imbalance
Right now, branchy has a funny imbalance: Each branch can fetch an instruction word every cycle, but only 6 processes can fetch arbitrary memory. So effectively, 128 RAM accesses per cycle are being eaten by CPU fetching, but a only 6 random accesses happen per cycle. How can this make sense?

A pipeline would help. Basically, branches can fetch sequential memory in one cycle and cache it, so that they never have to wait. This way, branching becomes more expensive other than tight loops.

With these limits in place, the CPU could likely be higher clocked without issue.

