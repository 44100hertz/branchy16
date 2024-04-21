#include "ppu.h"
#include "tests.h"

int main(int _argc, char **_argv) {
#ifdef TESTING
    run_tests();
#else
    ppu_init();
#endif
}
