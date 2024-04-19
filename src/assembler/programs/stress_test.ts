import { Op, BinOp, UnOp, Cond } from "~/assembler/cpuConsts";
import { type Asm, instruction, ident, reg, immed, num } from "~/assembler/definitions";

let program: Asm = [];

program.push(instruction({
  label: "start",
  op: BinOp.Add,
  dest: reg(0),
  arg0: reg(0),
  arg1: immed(111),
}));

program.push(instruction({
  op: Op.Branch,
  arg0: ident('start'),
  arg1: reg(0),
}));

program.push(instruction({
  op: Op.Compare,
  arg0: reg(0),
  arg1: immed(0xf000),
}));

program.push(instruction({
  op: Op.Jump,
  arg0: ident('skipstore'),
  compare: Cond.Gte,
}));

program.push(instruction({
  op: Op.Store,
  arg0: reg(0),
  arg1: reg(0),
  relative: false,
}));

program.push(instruction({
  label: "skipstore",
  op: BinOp.Add,
  dest: reg(0),
  arg0: reg(0),
  arg1: reg(0),
}));

program.push(instruction({
  op: Op.Load,
  arg0: reg(0),
  arg1: reg(0),
  relative: true,
  wait: false,
}));

program.push(instruction({
  op: Op.Jump,
  arg0: ident('start'),
  compare: Cond.Always,
}));

export default program;
