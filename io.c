//
// Created by Mario on 10/06/2024.
//

#include <stdlib.h>
#include <time.h>
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

/**
 * Print text and fill with whitespace to the right
 */
void printSpaceR(char *text, unsigned int whiteFill) {
    unsigned int nWhite = whiteFill - strlen(text);
    printf("%s%*c", text, nWhite, ' ');
}

void printSpaceL(char *text, unsigned int whiteFill) {
    unsigned int nWhite = whiteFill - strlen(text);
    printf("%*c%s", nWhite, ' ', text);
}

void printIntArray(unsigned int count, int val[count]) {
    for (unsigned int i = 0; i < count; i++) {
        printf("%d  ", val[i]);
    }
}