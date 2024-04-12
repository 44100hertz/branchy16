enum {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    IMMED,
    CONST_0,
    CONST_1,
    CONST_MINUS_1,
};

// combinations of conditionals
enum {
    COND_NEVER = 0b000,
    COND_EQ = 0b001,
    COND_NEQ = 0b110,
    COND_LT = 0b010,
    COND_GT = 0b100,
    COND_LTE = 0b011,
    COND_GTE = 0b101,
    COND_ALWAYS = 0b111,
};
