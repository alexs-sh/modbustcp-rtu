#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _STDARG_H
#include <stdarg.h>
#endif

#include "trace.h"

#include "cmd_server.h"

void trace_out(int force, char* color, const char* format, ...) {
  common_info_t* info = get_common_info();
  va_list args;

  /*assert(info);
  assert(info->settings);*/

  // Передать серверу управления
  va_start(args, format);
  cmd_server_trace_out(force, format, args);
  va_end(args);

  if (info) {
    if (!(info->settings->trace || force)) return;
  }

  if (color) printf("%s", color);

  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  if (color) printf("%s", ANSI_COLOR_RESET);

  printf("\n");
}

void trace_buffer_out(char* color, uint8_t* buffer, uint16_t size) {
  uint16_t i;

  common_info_t* info = get_common_info();

  assert(info);
  assert(info->settings);
  assert(buffer);

  // Передать серверу управления
  cmd_server_trace_buffer_out(buffer, size);

  if (!info->settings->trace) return;

  if (color) printf("%s", color);

  for (i = 0; i < size; i++) {
    printf("%d ", buffer[i]);
  }

  printf("\n");
}
