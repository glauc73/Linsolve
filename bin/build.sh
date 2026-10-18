#!/bin/bash

gcc -std=c11 -g src/linsolve.c -Iinclude -lm -o bin/linsolve

if [ $? -ne 0 ]; then
    echo "Erro: compilacao falhou."
    exit 1
fi

./bin/linsolve "${@}"
