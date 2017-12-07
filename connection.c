#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif  // _STRING_H

#ifndef _STDARG_H
#include <stdarg.h>
#endif

#ifndef connection_H_INCLUDED
#include "connection.h"
#endif

/** @brief Функция чтения данных из подключения
 * @param conn - указатель на подключение
 * @return Возвращает результат вызова функции чтения
 */
int connection_read(connection_t* conn, ...) {
  va_list args;

  assert(conn);
  assert(conn->read_func);

  va_start(args, conn);

  conn->last_result = conn->read_func(conn, args);

  va_end(args);

  return conn->last_result;
}

/** @brief Функция записи данных в подключение
 * @param conn - указатель на подключение
 * @return Возвращает результат вызова функции записи
 */
int connection_write(connection_t* conn, ...) {
  va_list args;

  assert(conn);
  assert(conn->write_func);

  va_start(args, conn);

  conn->last_result = conn->write_func(conn, args);

  va_end(args);

  return conn->last_result;
}

/** @brief Функция закрытия подключения
 * @param conn - указатель на подключение
 * @return Возвращает результат вызова функции закрытия
 */
int connection_close(connection_t* conn, ...) {
  va_list args;

  assert(conn);
  assert(conn->close_func);

  va_start(args, conn);

  conn->last_result = conn->close_func(conn, args);

  va_end(args);

  return conn->last_result;
}

/** @brief Функция для получения данных, связанных с коннектом
 * @param conn - указатель на подключение
 * @return Возвращает указатель на связанные данные
 */
/*void * connection_get_info(connection_t * conn) {

    assert(conn);
    assert(conn->info);

    return conn->info;
}*/
