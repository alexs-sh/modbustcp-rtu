#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED

#include "global.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

void trace_out(int force, char* color, const char* format, ...);

void trace_buffer_out(char* color, uint8_t* buffer, uint16_t size);

#endif  // TRACE_H_INCLUDED
