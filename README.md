# What is Branchy16?

Branchy16 is a fantasy console with an emulated 16-bit CPU that may run many threads at a time. It is both a fun programming challenge and a legitimate ISA that could be put on hardware.

# How do I work this?

Install pnpm and emscripten.
```
git clone git@github.com:44100hertz/branchy16
pnpm -r install
pnpm -r --parallel dev
```
...Then open a browser to localhost:3000

NIXOS: look at `emulator/scripts/fix-emscripten-nix`. This is the kind of thing you want to do to fix emscripten's cache.

# Programming Guide

Branchy16 is WIP, so all of this can change. A full programming guide will be made as it solidifies. For details on the current implementation, look at the C source in emulator/.

## Clock speed

Branchy16 has a 153.6khz clock speed.

Clock speed is 8 cycles per 60hz scan line, calculated as 60fps * 320 scan lines * 8 cycles = 153.6khz.

With all available branches in action, branchy16 is running a rough equivalent to 37.5mhz.

## Addressing

Branchy16 is 16-bit. Addressing is 16 bits, every instruction is 16 bits, and every register is 16 bits. This limits address space to 128KiB, which is shared by every branch. The first 120KiB are general-purpose, and the last 8KiB are used for devices.

## Programs

Branchy16 boots by loading a 120KiB ROM and starting a single branch at the first word of memory. Execution ends when every branch is either halted or waiting.

## Branches

Branchy16 can run up to 256 branches in parallel.

Each branch in Branchy16 has 8 general-purpose registers, a program counter, and a base pointer, each 16-bit. The base pointer is used for optional relative addressing, which is good for object-oriented design. Every relative load and store offsets the memory target by the base pointer.

To start a branch, use the "branch" instruction with a target and a base pointer. To stop it, use "halt".

Branches store memory between cycles, meaning that every store will not affect loads until the next cycle. Multiple writes to the same memory location in one cycle OR each other.

Branches may share data more predictably using a load-wait instruction. The waiting branch will execute a load only after another branch stores to the target cell, on the next cycle as the store. In the case of multiple load-waits on the same cell, each store to a target cell will only unlock a single branch, allowing for easy creation of mutexes.

## I/O and Devices

The first 120KiB (Address 0x0000-0xEFFF) are RAM, the last 8KiB in addresses 0xF000-0xFFFF are I/O addresses.

Loading from an I/O address will access a device, which returns a word based on the given address and may have side effects. For timing-sensitive values such as screen refresh or user input, load-waiting an I/O address called a 'lock' is used. Unlike standard stores, stores from I/O devices will unlock every load-waited branch on the same cycle rather than the next. Load-waiting an untimed I/O address will instantly resolve a value and is the same as a regular load.

Storing to an I/O address will send that value to the device and may trigger side-effects.

### Console

Write to 0xf000 to write a character to the console.

### Screen and Timing

The PPU is a device at 0xf100-0xf1ff that is responsible for drawing to the screen. It shares RAM with the CPU, which it uses as input for drawing.

Its output resolution is 240x240 with a 320x320 scan region. Overall, the PPU draws 240 scanlines for 1920 CPU cycles, then draws nothing for 640 CPU cycles called VBLANK. Within each scanline, the PPU draws 240 pixels for 6 cycles, then draws nothing for 2 CPU cycles called HBLANK. Writes to the GPU are ignored while it is drawing, but can be performed during either VBLANK or HBLANK.

The VBLANK lock at 0xf100 can be load-waited for any number of branches to resume execution at the start of VBLANK. Likewise, the scanline counter will write at the start of any visible scanline 0-239, and after 6 cycles it gives way to HBLANK.

## Instruction set

Look at emulator/src/cpu.c for the instruction set. Semantics for the assembler have not yet been decided, so it is hard to describe current operations.

# Contributions and License

This project is open-source under AGPL, meaning that any modifications must be open-sourced under AGPL, including forks that are hosted online.

You're free to contribute to the main project, and the suggested starting point for contributions is TODO.org. For any pull requests, please edit TODO.org accordingly. If you have ideas, submit a PR that modifies TODO.org and we can talk about it.

Thank you.
