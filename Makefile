debug:
	clang *.c -std=c2x -Wall -g -O0 -o branchy

release:
	clang *.c -std=c2x -O3 -o branchy

run: release
	./branchy

test:
	clang *.c -std=c2x -Wall -DTESTING -o branchy
	./branchy

.PHONY: all
