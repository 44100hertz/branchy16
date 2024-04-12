#!/bin/sh

export CFLAGS="-g"
export WASM_CMD="emmake make wasm"
$WASM_CMD
simple-file-watch --path="src" --command="$WASM_CMD"
