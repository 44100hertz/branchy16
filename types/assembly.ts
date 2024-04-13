import type { Op, BinOp, UnOp, Cond } from "./consts";

export type Asm = Line[];

export type Line = { label?: string, comment?: string } & ({
  kind: 'code',
  instruction?:
  { op: Op.Load, dest: Reg, src: Arg, indirect: boolean, relative: boolean, wait: boolean } |
  { op: Op.Store, dest: Arg, src: Reg, indirect: boolean, relative: boolean } |
  { op: Op.Jump, dest: Arg, src: Reg, indirect: boolean, relative: boolean, compare: Cond } |
  { op: Op.Branch, dest: Arg, } |
  { op: Op.Compare, src0: Reg, src1: Arg } |
  { op: Op.Putc, src: Arg } |
  { op: Op.Halt } |
  { op: BinOp, dest: Arg, src0: Arg, src1: Arg } |
  { op: UnOp, dest: Arg, src: Arg },
} | {
  kind: 'dataString',
  value: string,
});

export type Reg = { kind: 'reg', reg: 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 };
export type Arg = Reg | { kind: 'immed', num: number };
