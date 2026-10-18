#!/bin/bash

gcc -std=c11 src/main.c -Iinclude -lm -o linsolve

if [ $? -ne 0 ]; then
    echo "Erro: compilacao falhou."
    exit 1
fi

./linsolve "${@}"
