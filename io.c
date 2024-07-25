//
// Created by Mario on 10/06/2024.
//

#include "io.h"
#include <stdio.h>
#include <string.h>

char i_lineBuf[I_LINEBUF_LEN];

static unsigned short i_space = 1;

void printPrompt() {
    printf("> ");
}

/**
 * Consumes every character until newline from stdin.
 * Will block until a newline is input.
 */
void consumeLine() {
    char buf[100];
    char lastChar;
    do {
        fgets(buf, 100, stdin);
        lastChar = buf[strlen(buf) - 1];
    } while (lastChar != 10);
}

/**
 * Print text and fill up remaining mio_pSpace with whitespace
 */
void printSpaceL(char *text, unsigned int totalLen) {
    unsigned int nWhite = totalLen - strlen(text);
    printf("%s%*c", text, nWhite, ' ');
}

/**
 * Print text and align it to the right by filling in whitespaces
 */
void printSpaceR(char *text, unsigned int totalLen) {
    unsigned int nWhite = totalLen - strlen(text);
    printf("%*c%s", nWhite, ' ', text);
}

void printIntArray(unsigned int count, int val[count]) {
    for (unsigned int i = 0; i < count; i++) {
        printf("%d  ", val[i]);
    }
}

char *promptLine() {
    printPrompt();
    char *ret = fgets(i_lineBuf, I_LINEBUF_LEN, stdin);
    if (ret == NULL || ferror(stdin))
        return NULL;
    if (i_lineBuf[strlen(i_lineBuf) - 1] != '\n')
        consumeLine();
    return i_lineBuf;
}

void mio_setSpace(unsigned short space) {
    i_space = space;
}

void mio_pSpace() {
    printf("%*c", i_space, ' ');
}