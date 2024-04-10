// create a js-based "putchar" function for various purposes

#ifdef __EMSCRIPTEN__
#undef putchar
#include <emscripten.h>
EM_JS(int, putchar, (int c), {
    // write character w/o newline
    process.stdout.write(String.fromCharCode(c));
    return c;
})
#endif
