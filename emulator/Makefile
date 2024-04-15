OUTDIR=./bin/
OUT=./bin/branchy
CC?=$(CC)
CFLAGS:=$(CFLAGS) -std=c2x
SOURCES=src/*.c

EINCLUDES=-sEXPORTED_FUNCTIONS="\
					_cpu_init,_cpu_step,\
					_io_load,_io_store,\
					_set_peek_callback,_set_poke_callback,\
					_write_branching_hello,\
					_main"\
					-sEXPORTED_RUNTIME_METHODS="cwrap,addFunction"\
					-sALLOW_TABLE_GROWTH

outdir:
	mkdir -p $(OUTDIR)

test: outdir
	$(CC) $(SOURCES) $(CFLAGS) -g -Wall -DTESTING -o $(OUT)
	$(OUT)

wasm: outdir
	$(CC) $(SOURCES) $(CFLAGS) $(EINCLUDES) -o ./bin/index.mjs

clean:
	rm -f ./bin/index.mjs
	rm -f ./bin/index.wasm
	rm -f ./bin/branchy

.PHONY: outdir test wasm clean
