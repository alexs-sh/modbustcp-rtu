#ifndef CMD_SERVER_H_INCLUDED
#define CMD_SERVER_H_INCLUDED


#ifndef GLOBAL_H_INCLUDED
#include "global.h"
#endif

void cmd_server_create(common_info_t * info);

void cmd_server_trace_out(int force, char * color, const char * format, ...);

void cmd_server_trace_buffer_out(char * color, uint8_t * buffer, uint16_t size);

#endif // CMD_SERVER_H_INCLUDED
