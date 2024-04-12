OUTDIR=./bin/
OUT=./bin/branchy
CC?=$(CC)
CFLAGS:=$(CFLAGS) -std=c2x
SOURCES=src/*.c

EINCLUDES=-sEXPORTED_FUNCTIONS="_cpu_init,_cpu_step,_override_putchar,_write_branching_hello,_main" -sEXPORTED_RUNTIME_METHODS="cwrap,addFunction" -sALLOW_TABLE_GROWTH

outdir:
	mkdir -p $(OUTDIR)

test: outdir
	$(CC) $(SOURCES) $(CFLAGS) -Wall -DTESTING -o $(OUT)
	$(OUT)

wasm: outdir
	$(CC) $(SOURCES) $(CFLAGS) $(EINCLUDES) -o ./bin/branchy.mjs

clean:
	rm -r ./bin/

.PHONY: outdir test wasm clean
