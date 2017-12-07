#ifndef HIGH_CLIENT_H_INCLUDED
#define HIGH_CLIENT_H_INCLUDED

#ifndef _STDARG_H
#include <stdarg.h>
#endif

#ifndef GLOBAL_H_INCLUDED
#include "global.h"
#endif

#ifndef connection_H_INCLUDED
#include "connection.h"
#endif

#ifndef MESSAGE_H_INCLUDED
#include "message.h"
#endif

typedef struct high_client_info_t high_client_info_t;


/**@struct high_client_info_t
* Структура для хранения данных по каждому tcp-подключению
*/
struct high_client_info_t
{
    connection_t * client;          ///< указатель на коннект - обработчик событий

    struct timeval last_update;     ///< время последнего обновления

//    struct server_info_t * server_info;
};


int high_client_read(connection_t * conn, va_list args);

void high_client_write(connection_t * conn, va_list args);

int high_client_close(connection_t * conn, va_list args);

void high_client_process_message(message_t * message);

#endif // HIGH_CLIENT_H_INCLUDED
