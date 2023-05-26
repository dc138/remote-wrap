#!/bin/bash

cd $(dirname -- "$0")/..

echo Compiling sample
gcc -O2 -flto -std=c11 -o data/sample data/sample.c

echo Encrypting sample
make RUNFLAGS="../../../data/sample ../../../data/sample.enc" MAIN_USED=encrypt run
