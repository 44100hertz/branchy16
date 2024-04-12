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

## Essentials

Branchy16 is WIP, so all of this can change. A full programming guide will be made as it solidifies. For details on the current implementation, look at the C source in emulator/.

Branchy16 is 16-bit. Addressing is 16 bits, every instruction is 16 bits, and every register is 16 bits. This limits address space to 131kb (2^16 16-bit program words), which is shared by every branch.

Branchy16 boots by loading a 131kb ROM and starting a single branch at the first word of memory. Execution ends when every branch is either halted or waiting.

Each branch in Branchy16 has 8 general-purpose registers, a program counter, and a base pointer, each 16-bit. The base pointer is used for optional relative addressing, which is good for object-oriented design. Every relative load and store offsets the memory target by the base pointer.

To start a branch, use the "branch" instruction with a target and a base pointer. To stop it, call "halt".

Branches store memory between cycles, meaning that every store will not affect loads until the next cycle. Multiple writes to the same memory location in one cycle OR each other.

Branches may share data more predictably using a load-wait instruction. The waiting branch will execute a load only after another branch stores to the target cell, on the same cycle as the store. In the case of multiple load-waits on the same cell, each store to a target cell will only unlock a single branch, allowing for easy creation of mutexes.

## I/O

Branchy16 can write one character at a time to the console.

## Instruction set

Look at emulator/src/cpu.c for the instruction set. Semantics for the assembler have not yet been decided, so it is hard to describe current operations.
