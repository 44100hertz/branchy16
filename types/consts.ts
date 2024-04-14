  // /!\ DO NOT MODIFY THIS FILE /!\
  //
  // It has been autogenerated from gen_enums.lua
  //
  // If you wish to change the instruction listing, edit gen_enums.lua
  // and then run gen_enums.lua with lua 5.2 or luajit.
  //
  export enum Op {
    Load = 0x0,
    Store = 0x1,
    Jump = 0x2,
    Branch = 0x3,
    Compare = 0x4,
    Unused_5 = 0x5,
    Peek = 0x6,
    Poke = 0x7,
    Unused_8 = 0x8,
    Unused_9 = 0x9,
    Unused_A = 0xA,
    Unused_B = 0xB,
    Unused_C = 0xC,
    Unused_D = 0xD,
    Unused_E = 0xE,
    Halt = 0xF,
};

export enum BinOp {
    Add = 0x10,
    Unused_11 = 0x11,
    Unused_12 = 0x12,
    Unused_13 = 0x13,
    Unused_14 = 0x14,
    Unused_15 = 0x15,
    Unused_16 = 0x16,
    Unused_17 = 0x17,
    Unused_18 = 0x18,
    Unused_19 = 0x19,
    Unused_1A = 0x1A,
    Unused_1B = 0x1B,
    Unused_1C = 0x1C,
    Unused_1D = 0x1D,
};

export enum UnOp {
    Copy = 0x0,
    Unused_1 = 0x1,
    Unused_2 = 0x2,
    Unused_3 = 0x3,
    Unused_4 = 0x4,
    Unused_5 = 0x5,
    Unused_6 = 0x6,
    Unused_7 = 0x7,
    Unused_8 = 0x8,
    Unused_9 = 0x9,
    Unused_A = 0xA,
    Unused_B = 0xB,
    Unused_C = 0xC,
    Unused_D = 0xD,
    Unused_E = 0xE,
    Unused_F = 0xF,
    Unused_10 = 0x10,
    Unused_11 = 0x11,
    Unused_12 = 0x12,
    Unused_13 = 0x13,
    Unused_14 = 0x14,
    Unused_15 = 0x15,
    Unused_16 = 0x16,
    Unused_17 = 0x17,
    Unused_18 = 0x18,
    Unused_19 = 0x19,
    Unused_1A = 0x1A,
    Unused_1B = 0x1B,
    Unused_1C = 0x1C,
    Unused_1D = 0x1D,
    Unused_1E = 0x1E,
    Unused_1F = 0x1F,
};

export enum Cond {
    Never = 0x0,
    Eq = 0x1,
    Lt = 0x2,
    Lte = 0x3,
    Gt = 0x4,
    Gte = 0x5,
    Neq = 0x6,
    Always = 0x7,
};

export enum ArgNibble {
    R0 = 0x0,
    R1 = 0x1,
    R2 = 0x2,
    R3 = 0x3,
    R4 = 0x4,
    R5 = 0x5,
    R6 = 0x6,
    R7 = 0x7,
    Immed = 0x8,
    Const_0 = 0x9,
    Const_1 = 0xA,
    Const_Minus_1 = 0xB,
    Unused_C_Arg = 0xC,
    Unused_D_Arg = 0xD,
    Unused_E_Arg = 0xE,
    Unused_F_Arg = 0xF,
};

