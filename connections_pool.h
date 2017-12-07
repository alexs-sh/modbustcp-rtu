#ifndef CONNECTIONS_POOL_H_INCLUDED
#define CONNECTIONS_POOL_H_INCLUDED

#ifndef GLOBAL_H_INCLUDED
#include "global.h"
#endif

#ifndef connection_H_INCLUDED
#include "connection.h"
#endif

typedef struct connections_pool_t connections_pool_t;

/**@struct connections_pool_t
* Структура для хранения и обработки множества коннектов.
* При создании структуры автоматически создается список пустых подключений,
* которые впоследствии могут быть использованы. Пустые и занятые коннекты
* расположены в разных списках для облегчения работы с ними. Уничтожения
* пула приводит к уничтожению всех объектов. При этом, если в коннекта есть своя
* функция закрытия, она будет вызвана перед уничтожением объекта.
*
*/
struct connections_pool_t{

    connection_t * free;    ///< указатель на множество свободных коннектов

    connection_t * busy;    ///< указатель на множество занятых коннектов

    uint8_t need_update;    ///< Флаг необходимости обновления

    uint8_t size;           ///< Текущеуу кол-во занятых подключений

    uint8_t max_size;       ///< Максимальное количество подключений


};

connections_pool_t * connections_pool_create(uint8_t nconn, uint16_t info_size);

void connections_pool_free(connections_pool_t * pool);

connection_t * connections_pool_get_free_obj(connections_pool_t * pool);

int connections_pool_release_obj(connections_pool_t * pool, connection_t * obj);

void connections_pool_invalidate_obj(connections_pool_t * pool, connection_t * obj);

uint32_t connections_pool_process(connections_pool_t * pool, uint32_t timeout_ms);

void connections_pool_to_fdset(connections_pool_t * pool, fd_set * set, int * max_fd);


#endif // CONNECTIONS_POOL_H_INCLUDED
