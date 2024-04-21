export type PokeCallback = (addr: number, value: number) => void;

export declare const MEMSIZE: number;
export declare const SCREEN_WIDTH: number;
export declare const SCREEN_HEIGHT: number;

export declare class Screen {
  constructor(canvas: HTMLCanvasElement);
  runFrame(): boolean;
}
export declare function wasmInit(): Promise<void>;
export declare function loadBinary(binary: Uint16Array): void;
export declare function loadDisplayBusyLoop(): void;
export declare function setPokeHandler(index: number, fn: PokeCallback): void;
export declare function step(count?: number): boolean;
export declare function ioStore(addr: number, value: number): void;
