#ifndef _ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef CONNECTIONS_H_INCLUDED
#include "connection.h"
#endif

#ifndef SOCKETS_H_INCLUDED
#include "sockets.h"
#endif

#ifndef UDP_SERVER_H_INCLUDED
#include "udp_server.h"
#endif

#ifndef MESSAGE_H_INCLUDED
#include "message.h"
#endif

#ifndef CONNECTIONS_POOL_H_INCLUDED
#include "connections_pool.h"
#endif

#ifndef LOW_CLIENT_H_INCLUDED
#include "low_client.h"
#endif

static int udp_server_read(connection_t *conn);
static int udp_server_write(connection_t *conn, va_list args);
static int udp_server_close(connection_t *conn);

/** @brief Функция для создания UDP подключения
 * @return Возвращает 1 в случае успеха.
 */
int udp_server_create() {
  int result = 0;
  common_info_t *info = get_common_info();
  connection_t *udp_server = 0;

  // выделить свободный коннект
  udp_server = connections_pool_get_free_obj(info->connections_pool);

  if (!udp_server) return 0;

  // создать udp сокет
  udp_server->fd =
      socket_create(SOCKET_UDP);  // socket_create_server(info->settings->ip,
                                  // info->settings->port);
  result =
      socket_bind(udp_server->fd, info->settings->ip, info->settings->port);
  socket_set_blockmode(udp_server->fd, SOCKET_NON_BLOCK);

  if (!result || udp_server->fd == INVALID_FD) goto error;

  //  связать файловый дескриптор с функциями - обработчиками
  udp_server->read_func = (conn_handler_t)udp_server_read;
  udp_server->close_func = (conn_handler_t)udp_server_close;
  udp_server->write_func = (conn_handler_t)udp_server_write;

  info->server_info = udp_server;

  return 1;

error:

  if (udp_server)
    connections_pool_release_obj(info->connections_pool, udp_server);

  return 0;
}

/** @brief Функция чтения данных из udp подключения
 * @param conn - указатель на udp - коннект
 * @return Возвращает 1 в случае успеха
 */
static int udp_server_read(connection_t *conn) {
  message_t input_msg = {0};

  int result = 0;
  uint8_t request[MESSAGE_BUFFER_SIZE] = {0};

  uint16_t port = 0;
  uint16_t *ptr = (uint16_t *)request;

  uint32_t ip_addr = 0;

  const common_info_t *info = get_common_info();

  // прочитать все данные из подключения
  if ((result = socket_read_udp_raw(conn->fd, ptr, MESSAGE_BUFFER_SIZE,
                                    &ip_addr, &port)) < MB_HEADER_SIZE) {
    return 0;
  }

  input_msg.pdu.id = ntohs(ptr[0]);
  input_msg.pdu.proto = ntohs(ptr[1]);
  input_msg.pdu.len = ntohs(ptr[2]);

  // printf("ID = %d PROTO = %d LEN = %d SIZE = %d\n", input_msg.pdu.id,
  // input_msg.pdu.proto, input_msg.pdu.len,
  // queue_get_size(&info->tty_info->input_queue));

  if (input_msg.pdu.proto != 0 || input_msg.pdu.len == 0 ||
      // длина тела сообщения должна включать как минимум 3 байта
      input_msg.pdu.len < 4 ||
      // длина тела сообщения = вся длина - длина заголовка
      input_msg.pdu.len != result - MB_HEADER_SIZE ||
      // 250 байт нагрузки + 7 заголовка (самый большой заголовок для функции
      // записи 16)
      input_msg.pdu.len > MAX_MESSAGE_SIZE + 2) {
    return 0;
  }

  memcpy(input_msg.pdu.data, ptr + 3, input_msg.pdu.len);

  /*int i;
  printf("\n");
  for(i = 0 ; i < input_msg.pdu.len; i++) {
      printf("%d ", input_msg.pdu.data[i]);
  }*/

  // Заполнить сообщение информацией для дальнейшей обработки
  message_link_udp_source(&input_msg, conn, ip_addr, port);
  message_make_timestamp(&input_msg);

  return connection_write(info->tty_info->tty, &input_msg) != 0;
}

/** @brief Функция записи данных в udp подключение
 * @param conn - указатель на udp - коннект
 * @return Возвращает 1 в случае успеха
 */
static int udp_server_write(connection_t *conn, va_list args) {
  int result = 0;
  uint8_t out_buffer[MESSAGE_BUFFER_SIZE] = {0};
  message_t *message = 0;

  message = va_arg(args, message_t *);

  assert(message);

  // Сформировать выходной буфер
  result = message_to_buffer(message, out_buffer, MESSAGE_BUFFER_SIZE);

  if (result)
    socket_write_udp_raw(message->src_connection->fd, out_buffer, result,
                         message->src_ip, message->src_port);

  /* int i;
   printf("Send to %d : %d \n",  message->src_ip, message->src_port);

   for(i = 0; i < result; i++)
       printf("%d ", out_buffer[i]);*/
  // socket_write_tcp(message->src_connection->fd, out_buffer, result);

  return 1;
}

/** @brief Функция закрытия udp подключения
 * @param conn - указатель на udp - коннект
 * @return Возвращает 1 в случае успеха
 */
static int udp_server_close(connection_t *conn) {
  assert(conn);

  socket_close(conn->fd);

  return 1;
}
