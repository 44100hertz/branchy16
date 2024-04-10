OUTDIR=./bin/
OUT=./bin/branchy
CC?=$(CC)
CFLAGS:=$(CFLAGS) -std=c2x

EINCLUDES=-sEXPORTED_FUNCTIONS="_cpu_init,_cpu_step" -sEXPORTED_RUNTIME_METHODS="cwrap"

outdir:
	mkdir -p $(OUTDIR)

debug: outdir
	$(CC) *.c $(CFLAGS) -Wall -g -O0 -o $(OUT)

release: outdir
	$(CC) *.c $(CFLAGS) -O3 -o $(OUT)

run: outdir release
	$(OUT)

test: outdir
	$(CC) *.c $(CFLAGS) -Wall -DTESTING -o $(OUT)
	$(OUT)

wasm outdir:
	$(CC) *.c $(CFLAGS) $(EINCLUDES) -o ./bin/branchy.mjs

.PHONY: outdir debug release run test
