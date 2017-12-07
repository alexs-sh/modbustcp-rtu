#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _UNISTD_H
#include <unistd.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifndef _TIME_H
#include <time.h>
#endif

#ifndef STD_INPUT_H_INCLUDED
#include "std_input.h"
#endif

#ifndef connection_H_INCLUDED
#include "connection.h"
#endif

#ifndef CONNECTIONS_POOL_H_INCLUDED
#include "connections_pool.h"
#endif

#ifndef QUEUE_H_INCLUDED
#include "input_queue.h"
#endif

#ifndef SERVER_H_INCLUDED
#include "tcp_server.h"
#endif

#ifndef SOCKETS_H_INCLUDED
#include "sockets.h"
#endif

#ifndef LOW_CLIENT_H_INCLUDED
#include "low_client.h"
#endif

#define STDIN_BUFFER_SIZE 16

static void print_info(common_info_t* info);

/** @brief Функция для чтения данных со стандартного ввода.
 *
 * @param conn - указатель на коннект.ки.
 * @return Возвращает 1 в случае успеха.
 */
static int stdin_read(connection_t* conn) {
  common_info_t* info = get_common_info();  // connection_get_info(conn);

  char buffer[STDIN_BUFFER_SIZE] = {0};
  int result = read(conn->fd, buffer, STDIN_BUFFER_SIZE - 1);

  if (result <= 0) return 0;

  buffer[result - 1] = '\0';

  if (strcmp(buffer, "quit") == 0) {
    info->work = 0;
    printf("Goodbye.\n");
  } else if (strcmp(buffer, "info") == 0)
    print_info(info);
  else if (strcmp(buffer, "trace") == 0)
    info->settings->trace ^= 0x1;
  else
    printf("Unknown command!\n");

  return 1;
}

/** @brief Функция для создания обработчика стандартного ввода.
 *
 * @param info - указатель на общую информационную структуру
 */
void stdin_create(common_info_t* info) {
  connection_t* in = 0;

  if (info->settings->daemon) return;

  in = connections_pool_get_free_obj(info->connections_pool);

  if (!in) return;

  in->read_func = (conn_handler_t)stdin_read;
  in->write_func = in->close_func = 0;
  //    in->info = info;
  in->fd = STDIN_FILENO;
}

/** @brief Функция для вывода инофрмации на экран.
 *
 * @param info - указатель на общую информационную структуру
 */
static void print_info(common_info_t* info) {
  int i = 0;
  int counter = 0;

  time_t tm;

  uint32_t ip_addr = 0;

  tcp_server_info_t* server_info = 0;

  static char buffer[255];

  assert(info);
  tm = time(0);
  printf("\n------- ModbusDriver Info -------\n");
  printf("\nTime:\t%s\n", ctime_r(&tm, buffer));
  printf("Server:\n");
  printf("\tLocal IP:%s\n", info->settings->ip);
  printf("\tLocal Port:%d\n", info->settings->port);
  printf("\tMode:%s\n", info->settings->udp ? "UDP" : "TCP");
  if (!info->settings->udp) {
    server_info = info->server_info;
    ;
    printf("\tDisconnect timeout, ms:%u\n",
           info->settings->disconnect_timeout_ms);
    printf("\tMaximum clients:%d\n", MAX_CONNECTIONS);
    printf("\tConnected clients:%d\n", server_info->nclients);
    for (i = 0, counter = 0; i < MAX_CONNECTIONS; i++) {
      if (server_info->clients_info[i].client) {
        counter++;
        ip_addr = socket_get_client_ip(server_info->clients_info[i].client->fd);
        ip_addr = htonl(ip_addr);
        printf("\t\t%d) %u.%u.%u.%u:%d\n", counter,

#ifdef MX_BIG_ENDIAN
               (ip_addr & 0xFF000000) >> 24, (ip_addr & 0x00FF0000) >> 16,
               (ip_addr & 0x0000FF00) >> 8, (ip_addr & 0x000000FF),
#else
               (ip_addr & 0x000000FF), (ip_addr & 0x0000FF00) >> 8,
               (ip_addr & 0x00FF0000) >> 16, (ip_addr & 0xFF000000) >> 24,
#endif
               socket_get_client_port(server_info->clients_info[i].client->fd));
      }
    }
  }

  printf("\nSerial:\n");
  printf("\tDevice:%s\n", info->settings->device);
  printf("\tSilence timeout, ms:%u\n", info->settings->silence_timeout_ms);
  printf("\tMaximum input queue size:%d\n", MAX_QUEUE_SIZE);
  printf("\tCurrent input queue size:%d\n",
         queue_get_size(&info->tty_info->input_queue));
  printf("\tValid frames:%.0f\n", info->tty_info->valid_frames);
  printf("\tDropped frames:%.0f\n", info->tty_info->dropped_frames);
  printf("\tInvalid frames:%.0f\n", info->tty_info->invalid_frames);
  printf("\tRX bytes:%.0f\n", info->tty_info->rx);
  printf("\tTX bytes:%.0f\n", info->tty_info->tx);
}
