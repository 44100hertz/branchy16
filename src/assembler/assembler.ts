import { Op } from "~/assembler/cpuConsts";
import type { Asm, Line, Arg, Reg, Num, Instruction, Ident } from "~/assembler/definitions";
import { ArgNibble } from "./cpuConsts";

// TODO: create source maps

export function assemble(program: Asm) {
  console.log(`Compiling program...`);

  let symbolTable = new SymbolTable();

  // Phase 1: name resolution.
  // Does not modify the program, only the symbol table.
  resolveRegisterAliases(program, symbolTable);
  resolveConstants(program, symbolTable);
  // Phase 2: assembly.
  // Map all identifiers to concrete values.
  const resolved = mapProgramToConcrete(program, symbolTable);
  // Build final binary.
  let binary = createBinary(resolved);

  return binary;
}

function resolveRegisterAliases(program: Asm, symbolTable: SymbolTable) {
  for (let i in program) {
    const line = program[i];
    if (line.kind == "regAlias") {
      symbolTable.defineRegAlias(line.alias, line.value.value);
    }
  }
}

export function binaryAsLittleEndian(binary: Uint16Array): Uint8Array {
  const out: number[] = [];
  for (let word of binary) {
    out.push(word & 0xff);
    out.push(word >> 8);
  }
  return new Uint8Array(out);
}

/// Multi-pass resolve all program constants and labels repeatedly, else throw.
function resolveConstants(program: Asm, symbolTable: SymbolTable) {
  let passes = 0;
  for (let num_unresolved = Infinity; num_unresolved > 0;) {
    let unresolved = resolveConstantsPass(program, symbolTable);
    if (unresolved.length == num_unresolved) {
      throw new Error(`Could not resolve identifiers ${unresolved}`);
    }
    num_unresolved = unresolved.length;
    ++passes;
  }
  console.log(`Resolved program in ${passes} passes.`)
}

/// Perform a single constant resolution pass.
/// Returns list of unresolved constants.
function resolveConstantsPass(program: Asm, sym: SymbolTable): string[] {
  let unresolved: string[] = [];

  let offset: number | undefined = 0;
  // Increment the assembler program counter, which may become undefined
  // in the case of unresolved names.
  let incOffset = (len: number | undefined) => {
    if (len === undefined) {
      offset = undefined;
    } else if (offset !== undefined) {
      offset += len;
    }
  }

  // TODO: create source maps
  // TODO: catch binary overwriting self earlier

  for (let i in program) {
    try {
      const line = program[i];
      switch (line.kind) {
        // special construct to change assembler target address
        case "setPc":
          const maybeOffset = sym.lookupConstant(line.value);
          if (maybeOffset === undefined) {
            unresolved.push(line.value.value as string);
          }
          offset = maybeOffset;
          break;
        case "constant":
          sym.defineConstant(line.alias, sym.lookupConstant(line.value));
          break;
        case "instruction": {
          sym.defineConstant(line.value.label, offset);
          const instr = line.value;
          const handleArg = (name: string) => {
            if (name in instr) {
              const argDef = (instr as any)[name];
              const arg = sym.lookupArg(argDef);
              if (arg === undefined) {
                // TODO: Handle failure case where undefined offset endsup
                // up being turned into constant nibble and actually has
                // a width of 0.
                unresolved.push(argDef.value);
                incOffset(1);
              } else {
                incOffset(argWidth(arg));
              }
            }
          }
          incOffset(1);
          handleArg('arg0');
          handleArg('arg1');
          break;
        }
        case "data":
          sym.defineConstant(line.label, offset);
          incOffset(line.value.length);
          break;
      }
    } catch (err: any) {
      throw new Error(`${i}: ${err}`);
    }
  }

  return unresolved;
}

function argWidth(arg: Arg): number {
  if (arg.kind == "reg") {
    return 0;
  }
  const num = arg.value;
  return (num === 0 || num === 1 || num === -1) ? 0 : 1;
}

function mapProgramToConcrete(program: Asm, sym: SymbolTable): Line<never>[] {
  return program.flatMap((line: Line<Ident>): Line<never>[] => {
    switch (line.kind) {
      case "setPc": return [{ ...line, value: { kind: 'num', value: sym.lookupConstantOrFail(line.value) } }]
      case "data": return [line];
      case "instruction": {
        const out = { ...line.value };
        if ('dest' in out) out.dest = { kind: "reg", value: sym.lookupRegOrFail(out.dest) };
        if ('arg0' in out) out.arg0 = sym.lookupArgOrFail(out.arg0);
        if ('arg1' in out) out.arg1 = sym.lookupArgOrFail(out.arg1);
        return [{ kind: 'instruction', value: out } as Line<never>];
      }
      default: return [];
    }
  })
}

function createBinary(program: Line<never>[]): Uint16Array {
  let offset = 0;
  let binary: number[] = [];

  const write = (words: number[]) => {
    for (let w of words) {
      if (offset in binary) {
        throw `overwriting existing binary at offset ${offset.toString(16)} (${offset})`;
      }
      binary[offset++] = w;
    }
  }

  program.forEach((line: Line<never>, lineno: number) => {
    try {
      switch (line.kind) {
        case "setPc":
          offset = line.value.value;
          break;
        case "data":
          write(line.value.split('').map(c => c.codePointAt(0) as number));
          break;
        case "instruction":
          write(assembleInstruction(line.value));
          break;
        case "constant":
        case "regAlias":
        case "empty":
        default:
          break;
      }
    } catch (err: any) {
      throw new Error(`${lineno}: ${err}`);
    }
  });

  // autofill zeroes
  let max_size = 0;
  for (let i = 0; i < (1 << 16); ++i) if (i in binary) max_size = i;
  for (let i = 0; i < max_size; ++i) if (!(i in binary)) binary[i] = 0;

  console.log(`Created ${binary.length} word binary`);

  return new Uint16Array(binary);
}

function assembleInstruction(instr: Instruction<never>): number[] {
  let word0 = 0;
  let immeds: number[] = [];

  let push_arg = (name: string, offset: number) => {
    if (name in instr) {
      let arg = getArgNibbleAndImmed((instr as any)[name]);
      word0 |= arg[0] << offset;
      if (arg[1] !== null) immeds.push(arg[1]);
    }
  }

  let setflag = (set: boolean, index: number) =>
    word0 |= set ? (1 << index) : 0;

  if ('unary' in instr) {
    word0 |=
      encodeUnary(instr.op) |
      instr.dest.value << 8;
    push_arg('arg0', 4);
  } else {
    word0 |= instr.op << 11;
    if ('dest' in instr) {
      word0 |= instr.dest.value << 8;
    }
    push_arg('arg0', 4);
    push_arg('arg1', 0);

    switch (instr.op) {
      case Op.Load:
        setflag(instr.wait, 10);
        setflag(instr.relative, 9);
        break;
      case Op.Store:
        setflag(instr.relative, 9);
        break;
      case Op.Jump:
        word0 |= instr.compare;
        break;
    }
  }

  return [word0, ...immeds];
}

function getArgNibbleAndImmed(arg: Arg): [number, number | null] {
  if (arg.kind == "reg") {
    return [arg.value, null];
  } else {
    const constNibble = {
      [0]: ArgNibble.Const_0,
      [1]: ArgNibble.Const_1,
      [-1]: ArgNibble.Const_Minus_1,
      [0xffff]: ArgNibble.Const_Minus_1,
    }[0 | arg.value];

    if (constNibble) {
      return [constNibble, null];
    }
    return [ArgNibble.Immed, arg.value];
  }
}

function encodeUnary(instr: number) {
  const UNARY_MASK = 0b11110 << 11;
  return UNARY_MASK | (instr >> 4 & 1) << 11 | (instr & 0xf);
}

class SymbolTable {
  constants: Record<string, number> = {};
  regAliases: Record<string, Reg["value"]> = {};

  constructor() { }

  private defineFunc<T>(table: Record<string, T>)
    : (name: string | undefined, value: T | undefined) => void {
    return function(name: string | undefined, value: T | undefined) {
      if (name === undefined || value === undefined) return;
      const prevDef = table[name];
      if (prevDef !== undefined && prevDef !== value) {
        throw `conflicting values for ${name}: ${prevDef}`;
      }
      table[name] = value;
    }
  }

  defineRegAlias = this.defineFunc(this.regAliases);
  defineConstant = this.defineFunc(this.constants);

  private lookupOrFail<I, O>(name: string, lookupFunc: (arg: Ident | I) => O | undefined)
    : (arg: Ident | I) => O {
    return function(input: Ident | I): O {
      const value = lookupFunc(input);
      if (value === undefined) {
        throw `expected ${name}, got <undefined>`;
      }
      return value;
    }
  }

  lookupConstant(addr: Num | Ident): number | undefined {
    if (addr.kind == "num") return addr.value;
    return this.constants[addr.value];
  }

  lookupConstantOrFail = this.lookupOrFail('constant', this.lookupConstant.bind(this));

  lookupReg(reg: Reg | Ident): Reg["value"] | undefined {
    if (reg.kind == "reg") return reg.value;
    return this.regAliases[reg.value];
  }

  lookupRegOrFail = this.lookupOrFail('register', this.lookupReg.bind(this));

  lookupArg(arg: Arg | Ident): Arg | undefined {
    if (arg.kind == "ident") {
      const reg = this.regAliases[arg.value];
      const num = this.constants[arg.value];
      if (reg && num) throw `Ambiguous argument ${arg.value}`;
      if (reg) return { kind: 'reg', value: reg };
      if (num) return { kind: 'num', value: num };
      return undefined;
    }
    return arg;
  }

  lookupArgOrFail = this.lookupOrFail('argument', this.lookupArg.bind(this));
}

