#!/bin/sh

export CFLAGS="-g -Og"
export WASM_CMD="emmake make wasm"
$WASM_CMD
if [[ -v WASM_WATCH ]]; then
	echo "Running watch server"
	simple-file-watch --path="src" --command="$WASM_CMD"
fi
