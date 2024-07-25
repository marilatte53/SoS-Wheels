//
// Created by Mario on 10/06/2024.
//

#ifndef IO_H
#define IO_H
#define I_LINEBUF_LEN 100

#include <stdint.h>

void consumeLine();

void printPrompt();

/**
 * Prompts the user and reads in a line into an internal buffer. Buffer size is fixed,
 * the rest of the line is discarded. 
 */
char *promptLine();

void printSpaceR(char *text, unsigned int totalLen);

void printSpaceL(char *text, unsigned int totalLen);

void printIntArray(unsigned int count, int val[count]);

void mio_setSpace(unsigned short space);

void mio_pSpace();

#endif //IO_H
