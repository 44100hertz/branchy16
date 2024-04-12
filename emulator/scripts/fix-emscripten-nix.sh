#!/bin/sh
export EM_CACHE=~/.config/emscripten-cache
mkdir -p $EM_CACHE
cp -r /nix/store/lnifipaygpzafi1jjy40f6igjh5sr3l7-emscripten-3.1.47/share/emscripten/cache $EM_CACHE
chmod u+rwX -R $EM_CACHE
