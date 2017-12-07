#ifndef	_STDLIB_H
#include <stdlib.h>
#endif



#ifndef CONNECTIONS_POOL_H_INCLUDED
#include "connections_pool.h"
#endif

/** @brief Функция для создания пула подключений
 * @param nconn - макс. кол-во подключений в пуле
 * @param info_size - размер области с информацией для каждого коннекта
 * @return Возвращает указатель на новый пул
 */
connections_pool_t * connections_pool_create(uint8_t nconn, uint16_t info_size)
{

    int i;
    connection_t * conn = 0;

    connections_pool_t * pool = calloc(1, sizeof(*pool));

    if(!pool)
        return 0;

    pool->free = pool->busy = 0;
    conn = 0;
    // создание пустых подключений и добавление их в список
    for(i = 0; i < nconn; i++)
    {

        //выделение одной области для коннекта и данных. Область данных идет сразу после инормации о коннекте
        //connection_t * new_connect = calloc(1, sizeof(*new_connect) + info_size);
        connection_t * new_connect = calloc(1, sizeof(*new_connect) + info_size);
        if(!new_connect)
            goto error;

        //new_connect->info_size = info_size;
        //new_connect->info = (uint8_t *)new_connect + sizeof(*new_connect);
        new_connect->pool = pool;

        //Первая запись?
        if(!pool->free)
        {
            pool->free = new_connect;
            conn = pool->free;
        }
        else
        {
            conn->next = new_connect;
            conn = conn->next;
        }

    }

    pool->size = 0;
    pool->max_size = nconn;
    pool->need_update = 0;

    return pool;

error:

    while(pool->free)
    {
        conn = pool->free->next;

        free(pool->free);

        pool->free = conn;
    }

    free(pool);

    return 0;
}


/** @brief Функция для уничтожения пула
 * @param pool - указатель на пул
 */
void connections_pool_free(connections_pool_t * pool)
{

    connection_t * next_conn = 0;
    connection_t * act_conn = 0;

    assert(pool);

    next_conn = pool->free;
    // удаление всех пустых коннектов
    while(next_conn)
    {

        act_conn = next_conn;
        next_conn = next_conn->next;
        free(act_conn);
    }

    // вызов закрывающей функции длвсех занятых коннектов
    next_conn = pool->busy;
    while(next_conn)
    {

        if(next_conn->close_func)
            connection_close(next_conn);
        next_conn = next_conn->next;
    }

    // освобождение памяти занятных коннектов
    next_conn = pool->busy;
    while(next_conn)
    {

        act_conn = next_conn;
        next_conn = next_conn->next;
        free(act_conn);
    }

    free(pool);
}


/** @brief Функция для получения пустого коннекта из пула.
 * 'Откусывает' первый коннект из списка свободных.
 * @param pool - указатель на пул
 * @return Возвращает указатель на свободный коннект. Если объектов нет, возвращает 0
 */
connection_t * connections_pool_get_free_obj(connections_pool_t * pool)
{

    connection_t * free_conn = 0;

    assert(pool);

    free_conn = pool->free;

    // не осталось пустых подключений
    if(!free_conn)
        return 0;

    // сместить начало списка на сл. элемент
    pool->free = pool->free->next;

    // переместить "ткушенный" коннект в начало списка занятых подключений
    free_conn->next = pool->busy;
    pool->busy = free_conn;

    pool->size++;

    // установить корректный статус отдаваемому коннекту
    free_conn->status = CONNECTION_STATUS_OK;
    return free_conn;

}

/** @brief Функция для освобождения коннекта и помещения его в список свободных.
 * @param pool - указатель на пул
 * @param obj - указатель на освобождаемый коннект
 * @return Возвращает 1 в случае успеха
 */
int connections_pool_release_obj(connections_pool_t * pool, connection_t * obj)
{

    connection_t * conn = 0;
    connection_t * prev_conn = 0;

    assert(pool);
    assert(obj);

    // найти коннект в списке занятых
    conn = pool->busy;

    while(conn)
    {

        if(conn == obj)
            break;

        prev_conn = conn;
        conn = conn->next;

    }

    // неверный адрес коннекта
    if(conn != obj)
        return 0;

    // если есть функция закрытия, то сначала вызвать её
    if(conn->close_func)
        connection_close(conn);

    // удалить заапись из списка занятых
    if(prev_conn)
        prev_conn->next = conn->next;
    else
        pool->busy = pool->busy->next;

    // поместить запись в начала списка свободных
    conn->next = pool->free;
    pool->free = conn;

    pool->size--;

    return 1;

}

/** @brief Функция, информирующая пул, что данный оннект больше не нужен и может юыть закрыт
 * @param pool - указатель на пул
 * @param obj - указатель на освобождаемый коннект
 */
void connections_pool_invalidate_obj(connections_pool_t * pool, connection_t * obj)
{

    assert(pool);
    assert(obj);
    obj->status = CONNECTION_STATUS_INVALID;
    pool->need_update = 1;

}

/** @brief Функция для перемещения файловых дескрипторов коннектов в структуру fd_set.
 * Необходима для работы с такими системынми функциями, как select и poll.
 * @param pool - указатель на пул
 * @param set - указатель на принимающую структуру fd_set
 * @param max_fd - указатель на переменную, которая будет хранить значения максимально большого дескриптора
 * @return Возвращает 1 в случае успеха
 */
void connections_pool_to_fdset(connections_pool_t * pool, fd_set * set, int * max_fd)
{

    connection_t * conn  = 0;

    assert(pool);
    assert(set);
    assert(max_fd);

    conn  = pool->busy;

    FD_ZERO(set);

    while(conn)
    {

        if(conn->fd != INVALID_FD)
        {
            FD_SET(conn->fd, set);

            if(*max_fd < conn->fd)
                *max_fd = conn->fd;
        }

        conn = conn->next;

    }

}

/** @brief Функция для обработки запросов на чтение всех коннектов из пула
 * @param pool - указатель на пул
 * @param timeout_ms - таймаут в миллисекундах на ожидание хотя-бы одноо активного соединения
 * @return В случае успеха возвращает кол-во миллимекунд, потраченных на ожидание активности
 */
uint32_t connections_pool_process(connections_pool_t * pool, uint32_t timeout_ms)
{



    fd_set set;

    int result = 0;
    int max_fd = INVALID_FD;

    uint32_t start_time = timeout_ms * MS;

    connection_t * conn = 0;
    connection_t * act_conn = 0;

    struct timeval timeout = mks_to_time(start_time);

    assert(pool);

    //printf("Process data\n");
    // есть ненужные коннекта?
    if(pool->need_update)
    {

        pool->need_update = 0;
        conn = pool->busy;

        // вычистить
        while(conn)
        {
            act_conn = conn;
            conn = conn->next;

            if(act_conn->status == CONNECTION_STATUS_INVALID)
                connections_pool_release_obj(pool, act_conn);

        }

    }

    // заполнить структура fd_set и проверить наличие активности
    connections_pool_to_fdset(pool, &set, &max_fd);

    result = select(max_fd + 1, &set, 0, 0, &timeout);

    // ошибка ожидания
    if(result < 0) {
        return 0;
    }
    // есть активные коннекты
    else if (result > 0)
    {

        // найти и обработать коннект
        conn = pool->busy;
        while(conn && result)
        {
            act_conn = conn;
            conn = conn->next;

            if(FD_ISSET(act_conn->fd, &set) &&
                    act_conn->read_func)
            {

                connection_read(act_conn);
                result--;
            }

        }

    }



    return (start_time - time_to_mks(timeout)) / MS;
}


