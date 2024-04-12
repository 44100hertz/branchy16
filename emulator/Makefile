OUTDIR=./bin/
OUT=./bin/branchy
CC?=$(CC)
CFLAGS:=$(CFLAGS) -std=c2x
SOURCES=src/*.c

EINCLUDES=-sEXPORTED_FUNCTIONS="_cpu_init,_cpu_step,_main" -sEXPORTED_RUNTIME_METHODS="cwrap"

outdir:
	mkdir -p $(OUTDIR)

debug: outdir
	$(CC) $(SOURCES) $(CFLAGS) -Wall -g -O0 -o $(OUT)

release: outdir
	$(CC) $(SOURCES) $(CFLAGS) -O3 -o $(OUT)

run: outdir release
	$(OUT)

test: outdir
	$(CC) $(SOURCES) $(CFLAGS) -Wall -DTESTING -o $(OUT)
	$(OUT)

wasm: outdir
	$(CC) $(SOURCES) $(CFLAGS) $(EINCLUDES) -o ./bin/branchy.mjs

clean:
	rm -r ./bin/

.PHONY: outdir debug release run test wasm
