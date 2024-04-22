import Module from "./bin/index";

export const MEMSIZE = 0xf000;
export const SCREEN_WIDTH = 240;
export const SCREEN_HEIGHT = 160;
const SCREEN_BUF_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * 4;

let cpu;
let _writebinary;

/// Load CPU from wasm. Run this before anything else, or it will break.
export async function wasmInit() {
  if (!cpu) {
    cpu = await Module();
    _writebinary = cpu.cwrap('cpu_write_binary', null, ['number', 'array']);
  }
}

/// Reset the CPU memory and execution state.
/// Then, load a binary, which is an array of 16-bit words, into the CPU.
export function loadBinary(binary) {
  cpu._cpu_init();
  console.assert(binary.length < MEMSIZE);

  function toLittleEndian(binary) {
    const out = [];
    for (let word of binary) {
      out.push(word & 0xff);
      out.push(word >> 8);
    }
    return out;
  }

  _writebinary(binary.length, toLittleEndian(binary));
}

export function loadDisplayBusyLoop() {
  cpu._cpu_init();
  cpu._write_display_busyloop();
}

const funcPointers = {};

/// Set up a CPU callback for device "poke" on address 0xf000 and up.
/// @param index device index, which routes pokes from 0xf?00
/// @param fn poke handler
export function setPokeHandler(index, fn) {
  console.assert(index < 0x10);
  if (!(fn in funcPointers)) {
    funcPointers[fn] = cpu.addFunction(fn, 'vii');
  }
  cpu._set_poke_callback(index, funcPointers[fn]);
}

/// Set up a CPU callback for device "peek" on address 0xf000 and up.
/// @param index device index, which routes peeks from 0xf?00
/// @param fn peek handler
export function setPeekHandler(index, fn) {
  console.assert(index < 0x10);
  if (!(fn in funcPointers)) {
    funcPointers[fn] = cpu.addFunction(fn, 'ii');
  }
  cpu._set_peek_callback(index, funcPointers[fn]);
}

/// Step the CPU by one or more cycles
export function step(count) {
  if (count !== undefined) {
    return cpu._cpu_step_multiple(count);
  } else {
    return cpu._cpu_step();
  }
}

/// Acting as an I/O device, store a word into CPU memory
export function ioStore(addr, value) {
  return cpu._io_store(addr, value);
}

export class Screen {
  context;
  imageData;

  constructor(canvas) {
    this.imageData = new ImageData(SCREEN_WIDTH, SCREEN_HEIGHT);
    this.context = canvas.getContext('2d');
  }

  runFrame() {
    const running = cpu._ppu_frame();
    const data = cpu._ppu_screen();
    // copy data from code to image buffer
    this.imageData.data.set(cpu.HEAPU8.subarray(data, data + SCREEN_BUF_SIZE));
    // draw image buffer
    this.context.putImageData(this.imageData, 0, 0);
    return running;
  }
}
