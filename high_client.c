#ifndef _ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifndef HIGH_CLIENT_H_INCLUDED
#include "high_client.h"
#endif

#ifndef CONNECTIONS_POOL_H_INCLUDED
#include "connections_pool.h"
#endif

#ifndef MESSAGE_H_INCLUDED
#include "message.h"
#endif

#ifndef LOW_CLIENT_H_INCLUDED
#include "low_client.h"
#endif

#ifndef SOCKETS_H_INCLUDED
#include "sockets.h"
#endif

#ifndef SERVER_H_INCLUDED
#include "tcp_server.h"
#endif

/** @brief Функция для чтения данных, поступивших с верхнего уровня
 * @param conn - указатель на обрабатываемый коннект
 * @return В случае успеха возвращает 1. Возвращает 0 - в случае ошибки чтения и
 * -1 в случае проблем с добавлением данных в обработчик послю порта
 */
int high_client_read(connection_t *conn, va_list args) {
  message_t input_msg = {0};

  uint16_t *ptr = (uint16_t *)&input_msg.pdu.data;

  const common_info_t *info = get_common_info();

  if (socket_read_tcp(conn->fd, ptr, MB_HEADER_SIZE) < MB_HEADER_SIZE) {
    return 0;
  }

  input_msg.pdu.id = ntohs(ptr[0]);
  input_msg.pdu.proto = ntohs(ptr[1]);
  input_msg.pdu.len = ntohs(ptr[2]);

  //    printf("ID = %d PROTO = %d LEN = %d SIZE = %d\n", input_msg.pdu.id,
  //    input_msg.pdu.proto, input_msg.pdu.len,
  //    queue_get_size(&info->tty_info->input_queue));

  if (input_msg.pdu.proto != 0 || input_msg.pdu.len == 0 ||
      // длина тела сообщения должна включать как минимум 3 байта
      input_msg.pdu.len < 4 ||
      // 250 байт нагрузки + 7 заголовка (самый большой заголовок для функции
      // записи 16)
      input_msg.pdu.len > MAX_MESSAGE_SIZE + 2) {
    return 0;
  }

  if (socket_read_tcp(conn->fd, ptr, input_msg.pdu.len) != input_msg.pdu.len) {
    return 0;
  }

  // Заполнить сообщение информацией для дальнейшей обработки
  message_link_tcp_source(&input_msg, conn);
  message_make_timestamp(&input_msg);

  // 2015.01.29 Приводит к закрытию подключения при переполнении очереди
  // return connection_write(info->tty_info->tty, &input_msg) != 0;

  connection_write(info->tty_info->tty, &input_msg);
  return 1;
}

/** @brief Функция закрытия коннекта с ВУ
 * @param conn - указатель на обрабатываемый коннект
 * @return В случае успеха возвращает 1.
 */
int high_client_close(connection_t *conn, va_list args) {
  assert(conn);

  socket_close(conn->fd);
  return 1;
}

/** @brief Функция для пересылки сообщения на ВУ
 * @param message - указатель на обрабатываемоу сообщение
 */
void high_client_write(connection_t *conn, va_list args) {
  int result = 0;
  uint8_t out_buffer[MESSAGE_BUFFER_SIZE] = {0};
  message_t *message = 0;

  message = va_arg(args, message_t *);

  assert(message);

  // Клиент все ещё подключен?
  if (!message_validate_source(message)) return;

  // Сформировать выходной буфер
  result = message_to_buffer(message, out_buffer, MESSAGE_BUFFER_SIZE);

  if (result) socket_write_tcp(message->src_connection->fd, out_buffer, result);
}
