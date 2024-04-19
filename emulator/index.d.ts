export type PokeCallback = (addr: number, value: number) => void;

export declare function wasmInit(): Promise<void>;
export declare function loadBinary(binary: Uint16Array): void;
export declare function loadDisplayBusyLoop(): void;
export declare function setPokeHandler(index: number, fn: PokeCallback): void;
export declare function step(count?: number): boolean;
export declare function ioLoad(addr: number): number;
export declare function ioStore(addr: number, value: number): void;
