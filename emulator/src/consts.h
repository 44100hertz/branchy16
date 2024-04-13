  // /!\ DO NOT MODIFY THIS FILE /!\
  //
  // It has been autogenerated from gen_enums.lua
  //
  // If you wish to change the instruction listing, edit gen_enums.lua
  // and then run gen_enums.lua with lua 5.2 or luajit.
  //
  enum Op {
    ITAG_LOAD = 0x0,
    ITAG_STORE = 0x1,
    ITAG_JUMP = 0x2,
    ITAG_BRANCH = 0x3,
    ITAG_COMPARE = 0x4,
    ITAG_PUTC = 0x5,
    ITAG_HALT = 0x6,
    ITAG_UNUSED_7 = 0x7,
    ITAG_UNUSED_8 = 0x8,
    ITAG_UNUSED_9 = 0x9,
    ITAG_UNUSED_A = 0xA,
    ITAG_UNUSED_B = 0xB,
    ITAG_UNUSED_C = 0xC,
    ITAG_UNUSED_D = 0xD,
    ITAG_UNUSED_E = 0xE,
    ITAG_UNUSED_F = 0xF,
};

enum BinOp {
    ITAG_ADD = 0x10,
    ITAG_UNUSED_11 = 0x11,
    ITAG_UNUSED_12 = 0x12,
    ITAG_UNUSED_13 = 0x13,
    ITAG_UNUSED_14 = 0x14,
    ITAG_UNUSED_15 = 0x15,
    ITAG_UNUSED_16 = 0x16,
    ITAG_UNUSED_17 = 0x17,
    ITAG_UNUSED_18 = 0x18,
    ITAG_UNUSED_19 = 0x19,
    ITAG_UNUSED_1A = 0x1A,
    ITAG_UNUSED_1B = 0x1B,
    ITAG_UNUSED_1C = 0x1C,
    ITAG_UNUSED_1D = 0x1D,
};

enum UnOp {
    ITAG_UNARY_COPY = 0x0,
    ITAG_UNARY_UNUSED_1 = 0x1,
    ITAG_UNARY_UNUSED_2 = 0x2,
    ITAG_UNARY_UNUSED_3 = 0x3,
    ITAG_UNARY_UNUSED_4 = 0x4,
    ITAG_UNARY_UNUSED_5 = 0x5,
    ITAG_UNARY_UNUSED_6 = 0x6,
    ITAG_UNARY_UNUSED_7 = 0x7,
    ITAG_UNARY_UNUSED_8 = 0x8,
    ITAG_UNARY_UNUSED_9 = 0x9,
    ITAG_UNARY_UNUSED_A = 0xA,
    ITAG_UNARY_UNUSED_B = 0xB,
    ITAG_UNARY_UNUSED_C = 0xC,
    ITAG_UNARY_UNUSED_D = 0xD,
    ITAG_UNARY_UNUSED_E = 0xE,
    ITAG_UNARY_UNUSED_F = 0xF,
    ITAG_UNARY_UNUSED_10 = 0x10,
    ITAG_UNARY_UNUSED_11 = 0x11,
    ITAG_UNARY_UNUSED_12 = 0x12,
    ITAG_UNARY_UNUSED_13 = 0x13,
    ITAG_UNARY_UNUSED_14 = 0x14,
    ITAG_UNARY_UNUSED_15 = 0x15,
    ITAG_UNARY_UNUSED_16 = 0x16,
    ITAG_UNARY_UNUSED_17 = 0x17,
    ITAG_UNARY_UNUSED_18 = 0x18,
    ITAG_UNARY_UNUSED_19 = 0x19,
    ITAG_UNARY_UNUSED_1A = 0x1A,
    ITAG_UNARY_UNUSED_1B = 0x1B,
    ITAG_UNARY_UNUSED_1C = 0x1C,
    ITAG_UNARY_UNUSED_1D = 0x1D,
    ITAG_UNARY_UNUSED_1E = 0x1E,
    ITAG_UNARY_UNUSED_1F = 0x1F,
};

enum Cond {
    COND_NEVER = 0x0,
    COND_EQ = 0x1,
    COND_LT = 0x2,
    COND_LTE = 0x3,
    COND_GT = 0x4,
    COND_GTE = 0x5,
    COND_NEQ = 0x6,
    COND_ALWAYS = 0x7,
};

enum ArgNibble {
    R0 = 0x0,
    R1 = 0x1,
    R2 = 0x2,
    R3 = 0x3,
    R4 = 0x4,
    R5 = 0x5,
    R6 = 0x6,
    R7 = 0x7,
    IMMED = 0x8,
    CONST_0 = 0x9,
    CONST_1 = 0xA,
    CONST_MINUS_1 = 0xB,
    UNUSED_C_ARG = 0xC,
    UNUSED_D_ARG = 0xD,
    UNUSED_E_ARG = 0xE,
    UNUSED_F_ARG = 0xF,
};

