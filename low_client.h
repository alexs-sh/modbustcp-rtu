#ifndef LOW_CLIENT_H_INCLUDED
#define LOW_CLIENT_H_INCLUDED

#ifndef _STDARG_H
#include <stdarg.h>
#endif

#ifndef GLOBAL_H_INCLUDED
#include "global.h"
#endif // GLOBAL_H_INCLUDED

#ifndef QUEUE_H_INCLUDED
#include "input_queue.h"
#endif // QUEUE_H_INCLUDED

#ifndef CONNECTIONS_H_INCLUDED
#include "connection.h"
#endif

#ifndef QUEUE_H_INCLUDED
#include "input_queue.h"
#endif

/*
#define tty_source_is_clear(info) ((info)->status == CLEAR)
#define tty_source_get_status(info) ((info)->status)
#define tty_source_to_fd(info)  ((info)->fd)

*/

typedef struct low_client_info_t low_client_info_t;

typedef enum {CLEAR, WAIT_ANSWER, READ_ANSWER} tty_status;  ///< Возможные состояние посл. порта (Пустой, ожидает ответа, читатет ответ)


/**@struct low_client_info_tа
 * Структура для хранения всех необходимой информации по последовательному порту и работы с ним.
 */
struct low_client_info_t{

    connection_t * tty;             ///< указатель на коннект - обработчик событий

    queue_t input_queue;            ///< входная очередь сообщений

    message_t current_message;      ///< обрабатываемое сообщение

    uint8_t current_slave;          ///< адрес текущего обрабатываемого slave-устройства

    uint8_t current_func;           ///< номер функции в текущем обрабатываемом запросе

    tty_status status;              ///< статус посл. порта

    struct timeval last_update;     ///< время посл. обновления порта

    float tx;                       ///< кол-во переданных байт
    float rx;                       ///< кол-во прочитанных байт

    float valid_frames;             ///< кол-во корректный сообщений
    float dropped_frames;           ///< кол-во отброшенных сообщений
    float invalid_frames;           ///< кол-во некорректных сообщений

};

int low_client_create();

int low_client_write(connection_t * conn, va_list args);

void low_client_invalidate_conn(low_client_info_t * tty, connection_t const *conn);

void low_client_check();

#endif // LOW_CLIENT_H_INCLUDED
