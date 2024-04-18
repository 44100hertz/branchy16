import type { Op, BinOp, UnOp, Cond } from "./cpuConsts";

export type Asm = Line<Ident>[];
export type Ident = { kind: 'ident', value: string };

// parameter T is set to "ident" for unresolved programs, and "never" for
// resolved programs to show that every value is concrete.

export type Line<T> = { comment?: string } & (
  { kind: 'instruction', value: Instruction<T>, } |
  { kind: "data", label?: string, value: string } |
  { kind: 'regAlias', alias: string, value: Reg } |
  { kind: "constant", alias: string, value: T | Num } |
  { kind: "setPc", value: T | Num } |
  { kind: "empty" }
);

export type Reg = { kind: 'reg', value: 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 };
export type Arg = Reg | Num;
export type Num = { kind: 'num', value: number, format?: 'decimal' | 'hex' | 'binary' };

export type Instruction<T> = { label?: string } & (
  { op: Op.Load, arg0: T | Arg, arg1: T | Arg, relative: boolean, wait: boolean } |
  { op: Op.Store, arg0: T | Arg, arg1: T | Arg, relative: boolean } |
  { op: Op.Jump, arg0: T | Arg, compare: Cond } |
  { op: Op.Branch, arg0: T | Arg, arg1: T | Arg } |
  { op: Op.Compare, arg0: T | Arg, arg1: T | Arg } |
  { op: Op.Halt } |
  { op: BinOp, dest: T | Reg, arg0: T | Arg, arg1: T | Arg } |
  { op: UnOp, unary: true, dest: T | Reg, arg0: T | Arg });

export function instruction(props: { label?: string, comment?: string } & Instruction<Ident>): Line<Ident> {
  const { comment, ...rest } = props;
  return ({
    kind: 'instruction',
    value: rest,
  })
}

export function ident(value: string): Ident {
  return { kind: 'ident', value };
}

export function reg(value: Extract<Reg, { kind: 'reg' }>["value"]): Reg {
  return { kind: 'reg', value };
}

export function immed(value: number | string): Ident | Arg {
  if (typeof value == "string") {
    return ident(value);
  } else {
    return num(value);
  }
}

export function num(value: number): Num {
  return {
    kind: 'num',
    value,
    format: 'hex',
  };
}
