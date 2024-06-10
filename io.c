//
// Created by Mario on 10/06/2024.
//

#include "io.h"
#include <stdio.h>
#include <string.h>

/**
 * Consumes every character until newline from stdin.
 * Will block until a newline is input.
 */
void consumeLine() {
    char buf[20];
    char lastChar;
    do {
        fgets(buf, 20, stdin);
        lastChar = buf[strlen(buf) - 1];
    } while (lastChar != 10);
}