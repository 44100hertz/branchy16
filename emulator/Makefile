OUTDIR=./bin/
OUT=./bin/branchy
CC?=$(CC)
CFLAGS:=$(CFLAGS) -std=c2x
SOURCES=src/*.c
TEST_SOURCES=src/cpu.c src/tests.c src/binarygen.c src/main.c

EINCLUDES=-sEXPORTED_FUNCTIONS="\
					_main,\
					_cpu_init,_cpu_write_binary,\
					_cpu_step,_cpu_step_multiple,\
					_ppu_frame,_ppu_screen,\
					_io_store,\
					_set_poke_callback,\
					_write_branching_hello,\
					_write_display_busyloop"\
					-sEXPORTED_RUNTIME_METHODS="cwrap,addFunction"\
					-sALLOW_TABLE_GROWTH

outdir:
	mkdir -p $(OUTDIR)

test: outdir
	$(CC) $(TEST_SOURCES) $(CFLAGS) -g -Wall -DTESTING -o $(OUT)
	$(OUT)

wasm: outdir
	$(CC) $(SOURCES) $(CFLAGS) $(EINCLUDES) -o ./bin/index.mjs

clean:
	rm -f ./bin/index.mjs
	rm -f ./bin/index.wasm
	rm -f ./bin/branchy

.PHONY: outdir test wasm clean
