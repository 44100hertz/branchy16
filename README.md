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

## Addressing

Branchy16 is 16-bit. Addressing is 16 bits, every instruction is 16 bits, and every register is 16 bits. This limits address space to 128KiB, which is shared by every branch. The first 120KiB are general-purpose, and the last 8KiB are used for devices.

## Programs

Branchy16 boots by loading a 120KiB ROM and starting a single branch at the first word of memory. Execution ends when every branch is either halted or waiting.

## Branches

Each branch in Branchy16 has 8 general-purpose registers, a program counter, and a base pointer, each 16-bit. The base pointer is used for optional relative addressing, which is good for object-oriented design. Every relative load and store offsets the memory target by the base pointer.

To start a branch, use the "branch" instruction with a target and a base pointer. To stop it, use "halt".

Branches store memory between cycles, meaning that every store will not affect loads until the next cycle. Multiple writes to the same memory location in one cycle OR each other.

Branches may share data more predictably using a load-wait instruction. The waiting branch will execute a load only after another branch stores to the target cell, on the same cycle as the store. In the case of multiple load-waits on the same cell, each store to a target cell will only unlock a single branch, allowing for easy creation of mutexes.

## I/O and Devices

The first 120KiB (Address 0x0000-0xEFFF) are RAM, the last 8KiB in addresses 0xF000-0xFFFF are I/O addresses.

Loading from an I/O address will access a device, which returns a word based on the given address and may have side effects. For timing-sensitive values such as screen refresh or user input, load-waiting an I/O address is used. Unlike standard stores, stores from I/O devices will unlock every load-waited branch. However, load-waiting an untimed I/O address will lock the branch forever.

Storing to an I/O address will send that value to the device and may trigger side-effects.

### Devices

Currently, the only device is character output. Every I/O store will write a character to the console.

## Instruction set

Look at emulator/src/cpu.c for the instruction set. Semantics for the assembler have not yet been decided, so it is hard to describe current operations.

### Devices

Writing to 0xF000 will output a character to the terminal.

# Contributions and License

This project is open-source under AGPL, meaning that any modifications must be open-sourced under AGPL, including forks that are hosted online.

You're free to contribute to the main project, and the suggested starting point for contributions is TODO.org. For any pull requests, please edit TODO.org accordingly. If you have ideas, submit a PR that modifies TODO.org and we can talk about it.

Thank you.
