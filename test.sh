#!/bin/bash

set -x

mkdir -p _build
pushd _build
cmake ..
make
popd
clang-10 -S -emit-llvm -o test.ll test.c
opt-10 -instnamer -load _build/*/*LVA* -lva test.ll > /dev/null
#rm -rf _build test.ll
