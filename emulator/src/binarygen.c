#include "binarygen.h"

#include "consts.h"
#include "cpu.h"

// write instruction word
#define WW(word) (cpu_store(offset, word), offset++)
// write special or binary word
#define WI(instr, n0, n1, n2) \
    (cpu_store(offset, (ITAG_##instr) << 11 | n0 << 8 | n1 << 4 | n2), offset++)
// write unary instruction word
#define WU(instr, n0, n1)                                                 \
    (cpu_store(offset,                                                    \
               cpu_encode_unary(ITAG_UNARY_##instr) | n0 << 8 | n1 << 4), \
     offset++)

void write_branching_hello() {
    word offset = 0;
    word char_ptr = 0x100;
    word writer_offset = 0x200;

    // hello, world branching program
    // which duplicates characters using a second branch
    //
    WI(BRANCH, 0b000, IMMED, IMMED);
    WW(writer_offset);
    WW(char_ptr);

    WU(COPY, R0, IMMED);
    word hello_offset = offset++;
    // loop:
    word loop_offset = WI(LOAD, 0, R1, R0);
    WI(COMPARE, 0, R1, CONST_0);
    WI(JUMP, 0, IMMED, COND_EQ);
    word done_offset = offset++;
    WI(STORE, 0, IMMED, R1);
    WW(char_ptr);
    WI(ADD, R0, R0, CONST_1);
    WI(JUMP, 0, IMMED, COND_ALWAYS);
    WW(loop_offset);
    // done:
    cpu_store(done_offset, offset);
    WI(HALT, 0, 0, 0);

    cpu_store(hello_offset, offset);
    char *hello = "hello, world\n";
    for (char *c = hello; *c != 0; ++c) WW(*c);
    WW(0);

    // second branch loop -- write chars twice
    offset = writer_offset;
    WI(LOAD, 0b110, R0, CONST_0);
    WI(STORE, 0, IMMED, R0);
    WW(0xf000);
    WI(STORE, 0, IMMED, R0);
    WW(0xf000);
    WI(JUMP, 0, IMMED, COND_ALWAYS);
    WW(writer_offset);
}

void write_hello() {
    word offset = 0;

    // hello, world program
    //
    WU(COPY, R0, IMMED);
    word hello_offset = offset++;
    // loop:
    word loop_offset = WI(LOAD, 0, R1, R0);
    WI(COMPARE, 0, R1, CONST_0);
    WI(JUMP, 0, IMMED, COND_EQ);
    word done_offset = offset++;
    WI(STORE, 0, IMMED, R1);
    WW(0xf000);
    WI(ADD, R0, R0, CONST_1);
    WI(JUMP, 0, IMMED, COND_ALWAYS);
    WW(loop_offset);
    // done:
    cpu_store(done_offset, offset);
    WI(HALT, 0, 0, 0);

    cpu_store(hello_offset, offset);
    char *hello = "hello, world\n";
    for (char *c = hello; *c != 0; ++c) WW(*c);
    WW(0);
}

void write_display_busyloop() {
    // set up tile pointers
    word offset = 0;
    WU(COPY, R3, CONST_0);           // scroll offset var
    WI(LOAD, 0b100, IMMED, IMMED);   // wait for vblank
    WW(0xf100);                      //
    WI(STORE, 0, IMMED, IMMED);      // store pattern offset
    WW(0xf120);                      //
    word pattern_offset = offset++;  //
    WI(STORE, 0, IMMED, IMMED);      // store attribute offset
    WW(0xf121);                      //
    word attrib_offset = offset++;

    // spam BG color writes
    word loop = WI(LOAD, 0b100, R1, IMMED);  // loop
    WW(0xf102);                              //
    WI(STORE, 0, IMMED, R0);                 // write background color
    WW(0xf10f);                              //
    WI(ADD, R0, R0, IMMED);                  // increment background color
    WW(0x0008);                              //
    WI(LOAD, 0, R2, IMMED);                  // load scanline counter
    WW(0xf103);                              //
    WI(COMPARE, 0, R2, IMMED);               // check for vblank
    WW(160);                                 //
    WI(JUMP, 0, IMMED, COND_LT);             // loop if not vblank
    WW(loop);

    // scroll diagonially
    WI(ADD, R3, R3, CONST_1);         // increment R3
    WI(STORE, 0, IMMED, R3);          // write to scroll X
    WW(0xf124);                       //
    WI(STORE, 0, IMMED, R3);          // write to scroll y
    WW(0xf125);                       //
    WI(JUMP, 0, IMMED, COND_ALWAYS);  // loop
    WW(loop);

    offset = 0x100;
    cpu_store(pattern_offset, offset);
    uint32_t pattern[16] = {
        // the letter B
        0x88888888,  //
        0x80188888,  //
        0x81288888,  //
        0x82345688,  //
        0x83488768,  //
        0x84588658,  //
        0x85676588,  //
        0x88888888,  //
        // smiley face
        0x88888888,  //
        0x88088088,  //
        0x88088088,  //
        0x88088088,  //
        0x88888888,  //
        0x80888808,  //
        0x88000088,  //
        0x88888888,  //
    };
    for (int i = 0; i < 32; ++i) {
        WW(pattern[i] >> 16);
        WW(pattern[i]);
    }

    cpu_store(attrib_offset, offset);
    for (int i = 0; i < 1024; ++i) {
        bool flipx = i % 2 == 0;
        bool flipy = (i / 2) % 2 == 0;
        byte pal = (i / 4) % 8;
        byte pat = (i / 32) % 2;
        WW(pat << 8 | pal << 4 | flipx << 3 | flipy << 2);
    }
}
