#ifndef tcp_server_H_INCLUDED
#define tcp_server_H_INCLUDED

#ifndef GLOBAL_H_INCLUDED
#include "global.h"
#endif

#ifndef connection_H_INCLUDED
#include "connection.h"
#endif

#ifndef HIGH_CLIENT_H_INCLUDED
#include "high_client.h"
#endif


typedef struct tcp_server_info_t tcp_server_info_t;

struct tcp_server_info_t
{
    connection_t * tcp_server;

    high_client_info_t * clients_info;

    int nclients;
};


int tcp_server_accept(connection_t * conn);

int tcp_server_close(connection_t * conn);

int tcp_server_create();

void tcp_server_check_clients(tcp_server_info_t * tcp_server_info, int size, uint32_t timeout_ms);

#endif // tcp_server_H_INCLUDED
