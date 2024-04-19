import { Op, BinOp, UnOp, Cond } from "~/assembler/cpuConsts";
import { type Asm, instruction, ident, reg, immed, num } from "~/assembler/definitions";

let program: Asm = [];

program.push({
  alias: 'channel_char',
  kind: 'constant',
  value: num(0x100),
  comment: 'Communication value for the two threads.'
});

program.push({
  alias: 'branch_writer',
  kind: 'constant',
  value: num(0x200),
  comment: 'Location of character writing routine'
});

program.push(instruction({
  op: Op.Branch,
  arg0: ident('branch_writer'),
  arg1: ident('channel_char'),
}));

program.push(instruction({
  op: UnOp.Copy,
  unary: true,
  dest: reg(0),
  arg0: ident('hello'),
}));

program.push(instruction({
  label: "send_char",
  op: Op.Load,
  arg0: reg(1),
  arg1: reg(0),
  relative: false,
  wait: false,
}));

program.push(instruction({
  op: Op.Compare,
  arg0: reg(1),
  arg1: immed(0),
}));

program.push(instruction({
  op: Op.Jump,
  arg0: ident('finish_writer'),
  compare: Cond.Eq,
}));

program.push(instruction({
  op: Op.Store,
  arg0: ident('channel_char'),
  arg1: reg(1),
  relative: false,
}));

program.push(instruction({
  op: BinOp.Add,
  dest: reg(0),
  arg0: reg(0),
  arg1: immed(1),
}));

program.push(instruction({
  op: Op.Jump,
  arg0: ident('send_char'),
  compare: Cond.Always,
}));

program.push(instruction({
  label: 'finish_writer',
  op: Op.Halt,
}));

program.push({
  label: 'hello',
  kind: 'data',
  value: 'hello, world\n',
})

program.push({
  kind: 'setPc',
  value: ident('branch_writer'),
});

program.push(instruction({
  label: "recieve_char",
  op: Op.Load,
  arg0: reg(0),
  arg1: immed(0),
  relative: true,
  wait: true,
}));

program.push(instruction({
  op: Op.Store,
  arg0: immed(0xf000),
  arg1: reg(0),
  relative: false,
}));

program.push(instruction({
  op: Op.Store,
  arg0: immed(0xf000),
  arg1: reg(0),
  relative: false,
}));

program.push(instruction({
  op: Op.Jump,
  arg0: ident('recieve_char'),
  compare: Cond.Always,
}));

export default program;
