#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdarg.h>

// File type for streams
typedef struct FILE FILE;

// Standard streams
extern FILE* stdin;
extern FILE* stdout; 
extern FILE* stderr;

// Basic printf family
int printf(const char* format, ...);
int fprintf(FILE* stream, const char* format, ...);
int sprintf(char* str, const char* format, ...);

// Basic puts
int puts(const char* str);

// Basic putchar
int putchar(int c);

// Basic input functions
char* fgets(char* str, int num, FILE* stream);

#endif // _STDIO_H
