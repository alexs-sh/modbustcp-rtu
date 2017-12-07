#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifndef _TIME_H
#include <time.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef CMD_SERVER_H_INCLUDED
#include "cmd_server.h"
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

#ifndef SERVER_H_INCLUDED
#include "tcp_server.h"
#endif

#ifndef SOCKETS_H_INCLUDED
#include "sockets.h"
#endif

#ifndef TRACE_H_INCLUDED
#include "trace.h"
#endif

#ifndef QUEUE_H_INCLUDED
#include "input_queue.h"
#endif

#ifndef LOW_CLIENT_H_INCLUDED
#include "low_client.h"
#endif

#define CMD_CLIENT_BUFFER_SIZE 256  // Буфер для чтения команд

// макрос для вывода строки через сокет
#define send_line(fd, bfr, size, res, txt)    \
  ({                                          \
    (res) = snprintf((bfr), (size), (txt));   \
    socket_write_tcp((fd), (bfr), (res));     \
    (bfr)[0] = 10;                            \
    socket_write_tcp((fd), (bfr), 1);         \
    (bfr)[0] = 13;                            \
    (res) = socket_write_tcp((fd), (bfr), 1); \
  })

// макрос для вывода строки с параметрами через сокет
#define send_line_args(fd, bfr, size, res, txt, ...)     \
  ({                                                     \
    (res) = snprintf((bfr), (size), (txt), __VA_ARGS__); \
    socket_write_tcp((fd), (bfr), (res));                \
    (bfr)[0] = 10;                                       \
    socket_write_tcp((fd), (bfr), 1);                    \
    (bfr)[0] = 13;                                       \
    (res) = socket_write_tcp((fd), (bfr), 1);            \
  })

static int cmd_server_free(connection_t* conn);

static int cmd_server_read(connection_t* conn);

static int cmd_client_read(connection_t* conn);

static int cmd_client_close(connection_t* conn);

static void print_info(common_info_t* info);

void cmd_server_create(common_info_t* info) {
  connection_t* cmd_server = 0;

  assert(info);

  // Если порт = 0, то нет необходимости создавать сервер
  if (!info->settings->cmd_srv_port) return;

  cmd_server = connections_pool_get_free_obj(info->connections_pool);

  if (!cmd_server) return;

  cmd_server->read_func = (conn_handler_t)cmd_server_read;
  cmd_server->write_func = 0;
  cmd_server->close_func = (conn_handler_t)cmd_server_free;
  // in->info = info;
  cmd_server->fd =
      socket_create_server(info->settings->ip, info->settings->cmd_srv_port);

  // Ошибка создания сервера
  if (cmd_server->fd == INVALID_FD) {
    connections_pool_invalidate_obj(info->connections_pool, cmd_server);
    trace_out(1, ANSI_COLOR_RED, "Can't create command server\n");
    return;
  }

  info->cmd_srv = cmd_server;
}

static int cmd_server_read(connection_t* conn) {
  // Подключение клиента управления
  common_info_t* info = 0;
  connection_t* connect = 0;
  int client = INVALID_FD;

  assert(conn);

  info = get_common_info();

  client = socket_accept(conn->fd, 0);

  // Клиент уже подключен. Закрыть новое подключение
  if (info->cmd_client) {
    socket_close(client);
    return 0;
  }

  // Получить объект из пула
  connect = connections_pool_get_free_obj(info->connections_pool);
  if (!connect) {
    socket_close(client);
    return 0;
  }

  // Связать файловый дескриптор с функциями - обработчиками и добавить
  // в в общее пользование
  connect->fd = client;
  connect->read_func = (conn_handler_t)cmd_client_read;    // high_client_read;
  connect->close_func = (conn_handler_t)cmd_client_close;  // high_client_close;
  connect->write_func = 0;

  info->cmd_client = connect;

  return 1;
}
static int cmd_server_free(connection_t* conn) {
  // Закрытие сервера и клиента управления
  common_info_t* info = 0;

  assert(conn);

  info = get_common_info();

  socket_close(info->cmd_srv->fd);
  info->cmd_srv = 0;

  return 1;
}

static int cmd_client_read(connection_t* conn) {
  common_info_t* info = get_common_info();  // connection_get_info(conn);

  char buffer[CMD_CLIENT_BUFFER_SIZE] = {0};
  int result = socket_read_tcp(conn->fd, buffer, CMD_CLIENT_BUFFER_SIZE - 1);

  // В случае фэйла закрыть коннект
  if (result <= 0) {
    connections_pool_invalidate_obj(conn->pool, conn);
    return 0;
  }

  // buffer[result - 1] = '\0';

  if (buffer[0] == 'i')
    print_info(info);
  else if (buffer[0] == 't')
    info->settings->net_trace ^= 0x1;
  else if (buffer[0] == 'h') {
    // #define send_line(fd,bfr,size,res,txt)
    send_line(conn->fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
              "Supported commands: info, trace, close, help\n");
  } else if (buffer[0] == 'c') {
    connections_pool_invalidate_obj(conn->pool, conn);
  } else {
    send_line(conn->fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
              "Unknown command.\nSupported commands: 'i' - info, 't' - trace,"
              "'c' - close, 'h' - help\n");
  }

  return 1;
}

static int cmd_client_close(connection_t* conn) {
  common_info_t* info = 0;

  assert(conn);

  info = get_common_info();

  socket_close(conn->fd);

  info->cmd_client = 0;

  return 1;
}

/** @brief Функция для вывода инофрмации на экран.
 *
 * @param info - указатель на общую информационную структуру
 */

static void print_info(common_info_t* info) {
  int i = 0;
  int counter = 0;
  int result = 0;
  int fd = INVALID_FD;

  time_t tm;

  uint32_t ip_addr = 0;

  tcp_server_info_t* server_info = 0;

  static char buffer[CMD_CLIENT_BUFFER_SIZE];
  static char time_buffer[64];

  assert(info);

  tm = time(0);
  fd = info->cmd_client->fd;

  send_line(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
            "\n------- ModbusDriver Info -------");

  ctime_r(&tm, time_buffer);
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result, "Time:\t%s",
                 time_buffer);

  send_line(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result, "Server:");

  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result, "\tLocal IP:%s",
                 info->settings->ip);
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result, "\tLocal Port:%d",
                 info->settings->port);
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result, "\tMode:%s",
                 info->settings->udp ? "UDP" : "TCP");

  if (!info->settings->udp) {
    server_info = info->server_info;

    send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
                   "\tDisconnect timeout, ms:%u",
                   info->settings->disconnect_timeout_ms);
    send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
                   "\tMaximum clients:%d", MAX_CONNECTIONS);
    send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
                   "\tConnected clients:%d", server_info->nclients);
    for (i = 0, counter = 0; i < MAX_CONNECTIONS; i++) {
      if (server_info->clients_info[i].client) {
        counter++;
        ip_addr = socket_get_client_ip(server_info->clients_info[i].client->fd);
        ip_addr = htonl(ip_addr);

#ifdef MX_BIG_ENDIAN
        send_line_args(
            fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
            "\t\t%d) %u.%u.%u.%u:%d", counter, (ip_addr & 0xFF000000) >> 24,
            (ip_addr & 0x00FF0000) >> 16, (ip_addr & 0x0000FF00) >> 8,
            (ip_addr & 0x000000FF),
            socket_get_client_port(server_info->clients_info[i].client->fd));
#else
        send_line_args(
            fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
            "\t\t%d) %u.%u.%u.%u:%d", counter, (ip_addr & 0x000000FF),
            (ip_addr & 0x0000FF00) >> 8, (ip_addr & 0x00FF0000) >> 16,
            (ip_addr & 0xFF000000) >> 24,
            socket_get_client_port(server_info->clients_info[i].client->fd));
#endif
      }
    }
  }

  send_line(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result, "Serial:");

  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result, "\tDevice:%s",
                 info->settings->device);
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
                 "\tSilence timeout, ms:%u",
                 info->settings->silence_timeout_ms);
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
                 "\tMaximum input queue size:%d", MAX_QUEUE_SIZE);
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
                 "\tCurrent input queue size:%d",
                 queue_get_size(&info->tty_info->input_queue));
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
                 "\tValid frames:%.0f", info->tty_info->valid_frames);
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
                 "\tDropped frames:%.0f", info->tty_info->dropped_frames);
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result,
                 "\tInvalid frames:%.0f", info->tty_info->invalid_frames);
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result, "\tRX bytes:%.0f",
                 info->tty_info->rx);
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result, "\tTX bytes:%.0f",
                 info->tty_info->tx);

  if (result <= 0)
    connections_pool_invalidate_obj(info->cmd_client->pool, info->cmd_client);
}

void cmd_server_trace_out(int force, char* color, const char* format, ...) {
  common_info_t* info = get_common_info();

  va_list args;

  int result = 0;

  int fd = INVALID_FD;

  static char buffer[CMD_CLIENT_BUFFER_SIZE];

  // нет глобальной инйы - рано выводить сообщения
  if (!info) return;

  // нет подключения - не выводим
  if (!info->cmd_client) return;

  // нет признака трассировки или важности сообщения - молчим
  if (!(info->settings->net_trace || force)) return;

  fd = info->cmd_client->fd;

  va_start(args, format);
  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result, format, args);
  va_end(args);

  if (result <= 0)
    connections_pool_invalidate_obj(info->cmd_client->pool, info->cmd_client);
}

void cmd_server_trace_buffer_out(char* color, uint8_t* buffer, uint16_t size) {
  uint16_t i;

  int result = 0;

  int buff_pos = 0;

  char* buff_ptr = 0;

  char* buff_end = 0;

  // Буфер для вывода сообщений. Выбран с запасом, т.к. все данные выводятся
  // в текстовом виде + дополняются пробелами
  static char str_buffer[CMD_CLIENT_BUFFER_SIZE * 4];
  static char out_buffer[CMD_CLIENT_BUFFER_SIZE * 4];

  common_info_t* info = get_common_info();

  assert(buffer);

  // нет глобальной инфы - рано выводить сообщения
  if (!info) return;

  // нет подключения - не выводим
  if (!info->cmd_client) return;

  // нет признака трассировки - молчим
  if (!info->settings->net_trace) return;

  // запретить использовать слишком большой размер выходных данных
  if (size > CMD_CLIENT_BUFFER_SIZE * 4) size = CMD_CLIENT_BUFFER_SIZE * 4;

  buff_ptr = str_buffer;
  buff_end = buff_ptr + CMD_CLIENT_BUFFER_SIZE * 4;

  // Сформировать выходное сообщение
  for (i = 0; i < size && buff_ptr < buff_end - 1; i++) {
    result = snprintf(buff_ptr, CMD_CLIENT_BUFFER_SIZE * 4 - buff_pos - 2,
                      "%d ", buffer[i]);

    if (result < 0) return;

    buff_pos += result;
    buff_ptr += result;
  }

  // Перевод на новую строку
  *buff_ptr = '\0';
  buff_pos++;
  //  send_line_args(fd, buffer, CMD_CLIENT_BUFFER_SIZE, result, format, args);

  send_line_args(info->cmd_client->fd, out_buffer, CMD_CLIENT_BUFFER_SIZE * 4,
                 i, "%s", str_buffer);

  if (result <= 0)
    connections_pool_invalidate_obj(info->cmd_client->pool, info->cmd_client);
}
