#ifndef _SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifndef _NETINET_IN_H
#include <netinet/in.h>
#endif

#ifndef _ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _UNISTD_H
#include <unistd.h>
#endif

#ifndef _FCNTL_H
#include <fcntl.h>
#endif

#ifndef _SYS_IOCTL_H
#include <sys/ioctl.h>
#endif  // _SYS_IOCTL_H

#ifndef _ERRNO_H
#include <errno.h>
#endif

#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif

#ifndef _ASSERT_H
#include <assert.h>
#endif  // _ASSERT_H

#ifndef SOCKETS_H_INCLUDED
#include "sockets.h"
#endif  // SOCKETS_H_INCLUDED

/** @brief Функция для создания сокета.
 *
 * @param type - тип сокета TCP / UDP. \т
 *  Допустимые значения: \n
 *      SOCKET_TCP - сокет для работы с TCP \n
 *      SOCKET_UDP - сокет для работы с UDP
 *
 * @return Возвращает дескриптор или -1 в случае неудачи.
 *
 */
int socket_create(int type) {
  if (type == SOCKET_UDP) return socket(PF_INET, SOCK_DGRAM, 0);
  return socket(PF_INET, SOCK_STREAM, 0);
}

/** @brief Фнукция для связываения сокета с портом и интерфейсом.
 *
 * @param s - дескриптор.
 * @param sock_addr - адрес интерфейса, к которому будет привязан сокет. Может
 * быть 0 (без привязки к конткретному интерфейсу), либо содержать ip - адрес
 * интерфейса, например 127.0.0.1.
 * @param port - номер порта, к которому будет привязан сокет
 * @return Возвращает 1 в случае успеха.
 *
 */
int socket_bind(int s, const char *sock_addr, uint16_t port) {
  int optval = 1;
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t len = sizeof(struct sockaddr_in);

  if (!SOCKET_VALIDATE(s)) return 0;

  memset(&addr, '\0', len);

  addr.sin_addr.s_addr = sock_addr ? inet_addr(sock_addr) : 0;
  addr.sin_family = PF_INET;
  addr.sin_port = htons(port);

  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) != 0)
    return 0;

  return bind(s, (struct sockaddr *)&addr, len) == 0;
}

/** @brief Функция для прослушивания сокета.
 *
 * @param s - дескриптор.
 * @return Возвращает 1 в случае успеха.
 *
 */
int socket_listen(int s) {
  const int BACKLOG = 5;

  if (!SOCKET_VALIDATE(s)) return 0;

  return listen(s, BACKLOG) == 0;
}

/** @brief Функция принимает подключение от удаленной машины.
 *
 * @param s - дескриптор.
 * @param client_addr - указатель на структура, которая будет заполнена данными
 * о подключаемой машине (ip, port). Может быть 0.
 * @return возвращает дескриптор нового подключения или -1.
 *
 */
int socket_accept(int s, struct sockaddr *client_addr) {
  int client = 0;
  struct sockaddr addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t len = sizeof(struct sockaddr);

  if (!SOCKET_VALIDATE(s)) return 0;

  client = accept(s, client_addr ? client_addr : &addr, &len);

  return client;
}

/** @brief Функция для подключения к удаленной машине. (Синхронная).\n
 * При вызове блокирует выполнение, пока не будет выполнено подключение, либо
 * время ожидания не превысит таймаут.
 * @param s - дескриптор.
 * @param srv_addr - ip - адрес или имя удаленной машины
 * @param srv_port - порт удаленной машины
 * @return Возвращает 1 в случае успеха.
 *
 */
int socket_connect(int s, const char *srv_addr, uint16_t srv_port) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t len = sizeof(struct sockaddr_in);

  assert(srv_addr);

  if (!SOCKET_VALIDATE(s)) return 0;

  addr.sin_addr.s_addr = inet_addr(srv_addr);
  addr.sin_family = PF_INET;
  addr.sin_port = htons(srv_port);

  return connect(s, (struct sockaddr *)&addr, len) == 0;
}

/** @brief Функция для закрытия сокета.
 *
 * @param s - дескриптор.
 *
 */
void socket_close(int s) {
  if (!SOCKET_VALIDATE(s)) return;

  shutdown(s, SHUT_RDWR);
  close(s);
}

/** @brief Функция для создания TCP сервера, связанного с заданным интефейсом и
 * портом.
 *
 * @param srv_addr - ip - адрес интерфейса. Может быть 0.
 * @param port - порт сервера.
 * @return Возвращает дескриптор сервера.
 *
 */
int socket_create_server(const char *srv_addr, uint16_t port) {
  int sock = socket_create(SOCKET_TCP);
  int optval = 1;

  if (sock == SOCKET_INVALID) return SOCKET_INVALID;

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) != 0)
    return SOCKET_INVALID;

  if (!socket_bind(sock, srv_addr, port)) return SOCKET_INVALID;

  if (!socket_listen(sock)) return SOCKET_INVALID;

  return sock;
}

/** @brief Функция для задания блокировки сокета.
 *
 * @param s - дескриптор.
 * @param mode - режим блокировки. \n
 *        Допустимые значения: \n
 *          SOCKET_BLOCK - сокет с блокировкой \n
 *          SOCKET_NON_BLOCK - сокет без блокировки.
 * @return Возвращает 1 в случае успеха.
 *
 */

int socket_set_blockmode(int s, int mode) {
  int flags = 0;

  if (!SOCKET_VALIDATE(s)) return 0;

  flags = fcntl(s, F_GETFL, 0);

  if (flags == -1) return 0;

  mode == SOCKET_NON_BLOCK ? (flags |= O_NONBLOCK) : (flags &= (~O_NONBLOCK));

  return fcntl(s, F_SETFL, flags) != -1;
}

/** @brief Функция для чтения режима блокировки с сокета.
 *
 * @param s - дескриптор.
 * @param mode - режим блокировки. \n
 *        Допустимые значения: \n
 *          SOCKET_BLOCK - сокет с блокировкой \n
 *          SOCKET_NON_BLOCK - сокет без блокировки.
 * @return Возвращает SOCKET_BLOCK или  SOCKET_NON_BLOCK.
 *
 */
int socket_get_blockmode(int s) {
  int flags = 0;

  if (!SOCKET_VALIDATE(s)) return 0;

  flags = fcntl(s, F_GETFL, 0);

  if (flags == -1) return 0;

  return (flags & O_NONBLOCK) ? SOCKET_NON_BLOCK : SOCKET_BLOCK;
}

/** @brief Функция для задания таймаутов сокета.
 *
 * @param s - дескриптор.
 * @param read_timeout_ms - таймаут на чтение, мс.
 * @param write_timeout_ms - таймаут на запись, мс.
 * @return Возвращает 1 в случае успеха.
 *
 */

int socket_set_timeouts(int s, uint32_t read_timeout_ms,
                        uint32_t write_timeout_ms) {
  const int MSEC = 1000;
  struct timeval timeout;
  memset(&timeout, 0, sizeof(timeout));
  int r_result = 1, w_result = 1;

  if (!SOCKET_VALIDATE(s)) return 0;

  if (read_timeout_ms) {
    timeout.tv_sec = read_timeout_ms / MSEC;
    timeout.tv_usec = read_timeout_ms - timeout.tv_sec * MSEC;
    r_result = (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                           sizeof(timeout)) == 0);
  }

  if (write_timeout_ms) {
    timeout.tv_sec = write_timeout_ms / MSEC;
    timeout.tv_usec = write_timeout_ms - timeout.tv_sec * MSEC;
    w_result = (setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                           sizeof(timeout)) == 0);
  }

  return r_result && w_result;
}

/** @brief Функция для асинхронного подключения к удаленной машине.
 * Дополнительно реализует набор проверок на специфические ошибки, возникающие
 * при асинхронном подключении.
 *
 * @param s - дескриптор.
 * @param srv_addr - ip - адрес интерфейса. Может быть 0.
 * @param port - порт сервера.
 * @return Возвращает 1 в случае успеха или необходимости подождать результат
 * подключения. Если связь с удаленной машиной однозначено невозможна,
 * возвращает 0.
 *
 */

int socket_connect_async(int s, const char *srv_addr, uint16_t srv_port) {
  int result = 0, block_mode = 0;
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t len = sizeof(struct sockaddr_in);

  if (!SOCKET_VALIDATE(s)) return 0;

  if (!srv_addr) return 0;

  block_mode = socket_get_blockmode(s);

  if (!socket_set_blockmode(s, SOCKET_NON_BLOCK)) return 0;

  addr.sin_addr.s_addr = inet_addr(srv_addr);
  addr.sin_family = PF_INET;
  addr.sin_port = htons(srv_port);

  result = connect(s, (struct sockaddr *)&addr, len);

  if (result < 0) {
    /* Подкючение в процессе выполнения - все ОК. Во всех остальных случаях -
     * ошибка.*/
    if (errno == EINPROGRESS) {
      return socket_set_blockmode(s, block_mode) ? 1 : 0;
    }
  } else {
    return socket_set_blockmode(s, block_mode) ? 1 : 0;
  }

  return 0;
}

/** @brief Функция для чтения длины данных в буфере.
 *
 * @param s - дескриптор.
 * @return Возвращает количество байт данных доступных для чтения.
 *
 */
int socket_get_input_data_len(int s) {
  int bytes;

  if (!SOCKET_VALIDATE(s)) return 0;

  ioctl(s, FIONREAD, &bytes);
  return bytes;
}

/** @brief Функция для чтения IP адреса пира. версия для IPv4.
 *
 * @param s - дескриптор.
 * @return Возвращает 4 -байтовое число содержащее адрес удаленной машины.
 *
 */
uint32_t socket_get_client_ip(int s) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t addr_len = sizeof(struct sockaddr_in);
  if (getpeername(s, (struct sockaddr *)&addr, &addr_len) != 0) return 0;

  return htonl(addr.sin_addr.s_addr);
}

/** @brief Функция для чтения порта пира.
 *
 * @param s - дескриптор.
 * @return Возвращает 2 - байтовое число содержащее подключенный порт на
 * удаленной машине.
 *
 */
uint16_t socket_get_client_port(int s) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t addr_len = sizeof(struct sockaddr_in);
  if (getpeername(s, (struct sockaddr *)&addr, &addr_len) != 0) return 0;

  return htons(addr.sin_port);
}

/** @brief Функция для чтения данных из tcp сокета
 * @param s - дескриптор.
 * @param dst - указатель на буфер - приемник
 * @param size - размер буфера
 * @return Возвращает кол - во прочитанных байт. В случае ошибки возвращает -1.
 */
int socket_read_tcp(int s, void *dst, uint16_t size) {
  assert(dst);

  if (!SOCKET_VALIDATE(s)) return 0;

  return recv(s, dst, size, MSG_NOSIGNAL);
}

/** @brief Функция для записи данных в tcp сокет
 * @param s - дескриптор.
 * @param src - указатель на буфер - источник
 * @param size - размер буфера
 * @return Возвращает кол - во записанных байт. В случае ошибки возвращает -1.
 *
 */
int socket_write_tcp(int s, const void *src, uint16_t size) {
  assert(src);

  if (!SOCKET_VALIDATE(s)) return 0;

  return send(s, src, size, MSG_NOSIGNAL);
}

/** @brief Функция для чтения данных из udp сокета
 * @param s - дескриптор.
 * @param dst - указатель на буфер - приемник
 * @param size - размер буфера
 * @param ip_addr - переменная, которая будет хранить ip - адрес источника
 * данных. Может быть 0.
 * @param port - переменная, которая будет хранить порт источника данных. Может
 * быть 0.
 * @return Возвращает кол - во прочитанных байт. В случае ошибки возвращает -1.
 */
int socket_read_udp_raw(int s, void *dst, uint16_t size, uint32_t *ip_addr,
                        uint16_t *port) {
  int result = 0;
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t len = sizeof(struct sockaddr_in);

  assert(dst);

  if (!SOCKET_VALIDATE(s)) return 0;

  result = recvfrom(s, dst, size, MSG_NOSIGNAL, (struct sockaddr *)&addr, &len);

  if (result >= 0) {
    if (ip_addr) *ip_addr = ntohl(addr.sin_addr.s_addr);

    if (port) *port = ntohs(addr.sin_port);
  }

  return result;
}

/** @brief Функция для записи данных в udp сокет
 * @param s - дескриптор.
 * @param src - указатель на буфер - источник
 * @param size - размер буфера
 * @param ip_addr - переменная, содержащая ip - адрес приемника
 * @param port - переменная, содержащая порт приемника
 * @return Возвращает кол - во записанных байт. В случае ошибки возвращает -1.
 *
 */
int socket_write_udp_raw(int s, const void *src, uint16_t size,
                         uint32_t ip_addr, uint16_t port) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t len = sizeof(struct sockaddr_in);

  assert(src);

  if (!SOCKET_VALIDATE(s)) return 0;

  addr.sin_addr.s_addr = htonl(ip_addr);  // inet_addr(ip_addr);
  addr.sin_family = PF_INET;
  addr.sin_port = htons(port);

  return sendto(s, src, size, MSG_NOSIGNAL, (struct sockaddr *)&addr, len);
}

/** @brief Функция для записи данных в udp сокет
 * @param s - дескриптор.
 * @param src - указатель на буфер - источник
 * @param size - размер буфера
 * @param ip_addr - строка с ip - адресом приемника
 * @param port - переменная, содержащая порт приемника
 * @return Возвращает кол - во записанных байт. В случае ошибки возвращает -1.
 *
 */
int socket_write_udp(int s, const void *src, uint16_t size, const char *ip_addr,
                     uint16_t port) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t len = sizeof(struct sockaddr_in);

  assert(src);
  assert(ip_addr);

  if (!SOCKET_VALIDATE(s)) return 0;

  addr.sin_addr.s_addr = inet_addr(ip_addr);
  addr.sin_family = PF_INET;
  addr.sin_port = htons(port);

  return sendto(s, src, size, MSG_NOSIGNAL, (struct sockaddr *)&addr, len);
}
/*----------- TEST ---------------------------------------------------------*/

#ifdef UTILS_TEST

/** @brief Блок тестов для модуля связи. Включается при помощи флага UTILS_TEST
 *
 */

void socket_tests() {
  debug_print(YELLOW, "\n------ START SOCKETS TEST --------\n");

  const int SRV_TEST_PORT = 8888;
  //    const int CLIET_TEST_PORT = 8888;
  const int ACCEPT_TIMEOUT = 5000;
  const int BUFFER_SIZE = 64;
  const char *SRV_TEST_HOST = "127.0.0.1";

  test_start("1.Create server socket");
  int srv_sock = socket_create(SOCKET_TCP);

  if (SOCKET_VALIDATE(srv_sock))
    test_ok();
  else
    test_failed();

  test_start("2.Bind socket");

  if (socket_bind(srv_sock, 0, SRV_TEST_PORT))
    test_ok();
  else
    test_failed();

  test_start("3.Listen socket");

  if (socket_listen(srv_sock))
    test_ok();
  else
    test_failed();

  test_start("4.Set timeout on server socket");

  if (socket_set_timeouts(srv_sock, ACCEPT_TIMEOUT, 0))
    test_ok();
  else
    test_failed();

  test_start("5.Create async socket");
  int asock = socket_create(SOCKET_TCP);

  if (SOCKET_VALIDATE(asock))
    test_ok();
  else
    test_failed();

  test_start("6.Bind async socket");
  if (socket_bind(asock, 0, 0))
    test_ok();
  else
    test_failed();

  test_start("7.Async connect");

  if (socket_connect_async(asock, SRV_TEST_HOST, SRV_TEST_PORT))
    test_ok();
  else
    test_failed();

  test_start("8.Accept connection");

  int client = socket_accept(srv_sock, 0);

  if (SOCKET_VALIDATE(client))
    test_ok();
  else
    test_failed();

  send(asock, "TEST MESSAGE", 12, 0);

  if (SOCKET_VALIDATE(client)) {
    unsigned char buffer[BUFFER_SIZE];
    int result;

    debug_print(MAGENTA, "\t-Len of data in buffer = %d\n",
                socket_get_input_data_len(client));
    while ((result = recv(client, buffer, BUFFER_SIZE, MSG_NOSIGNAL)) > 0) {
      debug_print(GREEN, "Get %d bytes\n", result);
      socket_close(asock);
    }

    debug_print(YELLOW, "Close Connection\n");
    socket_close(client);
  }
  socket_close(srv_sock);

  /* Just for lulz */
  /*int srv1 = socket_create_server("192.168.0.1", 8888);
  int srv2 = socket_create_server("192.168.0.2", 8888);


  printf("SRV 1 = %d, SRV 2 = %d\n", srv1, srv2);

  int c1 = socket_accept(srv1, 0);
  send(c1, "SASHA1", 6, 0);
  socket_close(c1);

  int c2 = socket_accept(srv2, 0);
  send(c2, "SASHA2", 6, 0);


  socket_close(c2);
  int in;
  scanf("%d", &in);

  socket_close(srv1);
  socket_close(srv2);*/
}
#endif
