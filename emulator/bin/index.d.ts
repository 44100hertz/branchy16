/// <reference types="emscripten" />

export type PeekCallback = (addr: number) => number;
export type PokeCallback = (addr: number, value: number) => void;

export interface Cpu extends EmscriptenModule {
  _cpu_init: () => void,
  _cpu_step: () => boolean,
  _cpu_step_multiple: (steps: number) => boolean,
  _io_load: (addr: number) => number,
  _io_store: (addr: number, value: number) => void,
  _set_poke_callback: (index: number, cb: number) => void,
  _write_branching_hello(): () => void,
  _write_display_busyloop(): () => void,

  addFunction: typeof addFunction,
  ccall: typeof ccall,
}

export default function em_main(): Promise<Cpu>;
