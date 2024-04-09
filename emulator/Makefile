OUTDIR=./bin/
OUT=./bin/branchy

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

.PHONY: outdir debug release run test
