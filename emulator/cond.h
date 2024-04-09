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
