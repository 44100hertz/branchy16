OUTDIR=./bin/
OUT=./bin/branchy
CC=clang

EINCLUDES="-sEXPORTED_FUNCTIONS=cpu_init,cpu_step -sEXPORTED_RUNTIME_METHODS=cwrap"

outdir:
	mkdir -p $(OUTDIR)

debug: outdir
	clang *.c -std=c2x -Wall -g -O0 -o $(OUT)

release: outdir
	clang *.c -std=c2x -O3 -o $(OUT)

run: outdir release
	$(OUT)

test: outdir
	clang *.c -std=c2x -Wall -DTESTING -o $(OUT)
	$(OUT)

wasm:
	emcc *.c -std=c2x -O3 $(EFLAGS) $(EINCLUDES) -o ../public/branchy.wasm -o ../public/branchy.mjs

.PHONY: outdir debug release run test
