#!/bin/sh
WASM_CMD="simple-file-watch --path='emulator' --comand='make -C emulator wasm && cp bin/'"
concurrently "simple-file-watch --path='emulator' --command='make -C emulator wasm'" "vinxi dev"
