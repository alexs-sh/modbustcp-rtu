#ifndef connection_H_INCLUDED
#define connection_H_INCLUDED

#ifndef _SYS_SELECT_H
#include <sys/select.h>
#endif

#ifndef GLOBAL_H_INCLUDED
#include "global.h"
#endif

#define CONNECTION_STATUS_OK 1

#define CONNECTION_STATUS_INVALID 2

//#define connection_get_info(conn)      (conn)->info    /**< Получить указатель
//на область данных */

//#define connection_next(conn)          (conn)->next    /**< Получить указатель
//на сл. элемент */

//#define connection_to_fd(conn)         (conn)->fd      /**< Получить файловый
//дескриптор */

typedef struct connection_t connection_t;

typedef int (*conn_handler_t)(connection_t* connect, ...);

/**@struct connection_t
 * Элемент списка подключений. Элементы могут объединятся в односвязный список.
 * Кажый элемент содержит связку из файлового дескриптора и данных, необходимых
 * для обработки дескриптора.
 */
struct connection_t {
  // void * info;                    /**< указатель на область данных */

  conn_handler_t read_func;

  conn_handler_t write_func;

  conn_handler_t close_func;

  struct connections_pool_t* pool;

  int fd; /**< файловый дескриптор */

  int last_result; /**< Результат вызова полседнего хендлера */

  // uint16_t info_size;             /**< размер области данных */

  uint8_t status; /**< статус подключения */

  struct connection_t* next; /**< указатель на сл. элемент */
};

int connection_read(connection_t* conn, ...);

int connection_write(connection_t* conn, ...);

int connection_close(connection_t* conn, ...);

// void * connection_get_info(connection_t * conn);

#endif  // connection_H_INCLUDED
