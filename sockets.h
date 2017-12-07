#ifndef SOCKETS_H_INCLUDED
#define SOCKETS_H_INCLUDED

#ifndef _SYS_SOCKET_H
#include <sys/socket.h>
#endif // _SYS_SOCKET_H

#ifndef GLOBAL_H_INCLUDED
#include "global.h"
#endif // GLOBAL_H_INCLUDED


#define SOCKET_UDP 1        /**< UDP - сокет */
#define SOCKET_TCP 2        /**< TCP - сокет */

#define SOCKET_BLOCK 1      /**< Блокируемый сокет */
#define SOCKET_NON_BLOCK 2  /**< Неблокируемый сокет */

#define SOCKET_INVALID  -1  /**< Неверный/пустой дескриптор*/
#define SOCKET_VALIDATE(s) ((s) > SOCKET_INVALID)   /**< Макрос для проверки сокета */

int socket_create(int type);
int socket_bind(int s, const char * sock_addr, uint16_t port);
int socket_listen(int s);
int socket_accept(int s, struct sockaddr * client_addr);
int socket_connect(int s, const char * srv_addr, uint16_t srv_port);
int socket_connect_async(int s, const char * srv_addr, uint16_t srv_port);
void socket_close(int s);
int socket_create_server(const char * srv_addr, uint16_t port);
int socket_set_blockmode(int s, int mode);
int socket_get_blockmode(int s);
int socket_set_timeouts(int s, uint32_t read_timeout, uint32_t write_timeout);
int socket_get_input_data_len(int s);

int socket_read_tcp(int s, void * dst, uint16_t size);
int socket_write_tcp(int s, const void * src, uint16_t size);


int socket_read_udp_raw(int s, void * dst, uint16_t size, uint32_t * ip_addr, uint16_t * port);
int socket_write_udp_raw(int s, const void * src, uint16_t size, uint32_t ip_addr, uint16_t port);
int socket_write_udp(int s, const void * src, uint16_t size, const char * ip_addr, uint16_t port);

uint32_t socket_get_client_ip(int s);
uint16_t socket_get_client_port(int s);

#ifdef UTILS_TEST
void socket_tests();
#endif

#endif // SOCKETS_H_INCLUDED
