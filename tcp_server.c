#ifndef	_STDLIB_H
#include <stdlib.h>
#endif

#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif

#ifndef tcp_server_H_INCLUDED
#include "tcp_server.h"
#endif

#ifndef HIGH_CLIENT_H_INCLUDED
#include "high_client.h"
#endif

#ifndef CONNECTIONS_POOL_H_INCLUDED
#include "connections_pool.h"
#endif

#ifndef SOCKETS_H_INCLUDED
#include "sockets.h"
#endif

#ifndef LOW_CLIENT_H_INCLUDED
#include "low_client.h"
#endif

#ifndef QUEUE_H_INCLUDED
#include "input_queue.h"
#endif

static int tcp_server_read_client(connection_t * conn);
static int tcp_server_close_client(connection_t * conn);
static void tcp_server_update_client_info(tcp_server_info_t * tcp_server, connection_t * client);

/** @brief Функция обработки запроса на подключение клиента
 * @param conn - указатель коннект сервера
 * @return Возвращает 1 в случае успеха.
 */
int tcp_server_accept(connection_t * conn)
{
    const common_info_t * info = 0;
    connection_t * connect = 0;
    tcp_server_info_t * server_info = 0;
    int client = 0;

    assert(conn);

    info = get_common_info();
    server_info = info->server_info;

    assert(server_info);

    client = socket_accept(conn->fd, 0);

    if(server_info->nclients >= MAX_CONNECTIONS)
    {
        socket_close(client);
        return 0;
    }

    connect = connections_pool_get_free_obj(info->connections_pool);
    if(!connect)
    {
        socket_close(client);
        return 0;
    }

    //  связать файловый дескриптор с функциями - обработчиками
    connect->fd = client;
    connect->read_func = (conn_handler_t)tcp_server_read_client;//high_client_read;
    connect->close_func = (conn_handler_t)tcp_server_close_client;//high_client_close;
    connect->write_func = (conn_handler_t)high_client_write;

    server_info->nclients++;

    tcp_server_update_client_info(server_info, connect);
    return 1;

}

/** @brief Функция для закрытия сервера
 * @param conn - указатель на коннект сервера
 * @return Возвращает 1 в случае успеха
 */
int tcp_server_close(connection_t * conn)
{

    const common_info_t * info = 0;
    tcp_server_info_t * server_info = 0;
    assert(conn);

    info = get_common_info();
    server_info = info->server_info;

    assert(server_info);

    socket_close(conn->fd);

    if(server_info->clients_info)
        free(server_info->clients_info);

    free(server_info);

    return 1;
}

/** @brief Функция для создания сервера
 * @return Возвращает 1 в случае успеха
 */
int tcp_server_create()
{

    connection_t * tcp_server = 0;

    tcp_server_info_t * server_info = 0;

    common_info_t * info = get_common_info();

    if(info->server_info)
        return 0;

    // выделить память для хранения данных сервера
    server_info = calloc(1, sizeof(tcp_server_info_t));

    if(!server_info)
        goto error;

    // выделить память для хранения данных клиентских подключений
    server_info->clients_info = calloc(MAX_CONNECTIONS, sizeof(high_client_info_t));

    if(!server_info->clients_info)
        goto error;


    // выделить свободный обработчик коннектов
    tcp_server = connections_pool_get_free_obj(info->connections_pool);

    if(!tcp_server)
        goto error;

    // создать прослушивающий сокет
    tcp_server->fd = socket_create_server(info->settings->ip, info->settings->port);

    if(tcp_server->fd == INVALID_FD)
        goto error;

    //  связать файловый дескриптор с функциями - обработчиками
    tcp_server->read_func = (conn_handler_t)tcp_server_accept;
    tcp_server->close_func =   (conn_handler_t)tcp_server_close;
    tcp_server->write_func = 0;


    server_info->tcp_server = tcp_server;
    info->server_info = server_info;


    return 1;

error:

    if(tcp_server)
        connections_pool_release_obj(info->connections_pool, tcp_server);

    if(server_info &&
            server_info->clients_info)
        free(server_info->clients_info);

    if(server_info)
        free(server_info);


    return 0;


}

/** @brief Функция для чтения данных от клиентского подключения
 * @param conn - указатель на коннект клиента
 * @return Возвращает 1 в случае успеха
 */
static int tcp_server_read_client(connection_t * conn)
{

    common_info_t * info = 0;

    assert(conn);

    info = get_common_info();
    va_list list;
    if(!high_client_read(conn, list))
    {
        // Повреждение всех заисей, связанных с закрываемым подключением
        //queue_invalidate_recs(&info->tty_info->input_queue, conn);

        // Уведомление посл. порта о закрытии подключения. Необходмо для очистки очереди
        // и т.п. дел
        low_client_invalidate_conn(info->tty_info, conn);
        connections_pool_invalidate_obj(info->connections_pool, conn);
        return 0;
    }

    tcp_server_update_client_info(info->server_info, conn);

    return 1;
}


/** @brief Функция для обновления метки времени клиента
 * @param tcp_server - сервер, к которому относится коннет
 * @param client - указатель на коннект
 */
static void tcp_server_update_client_info(tcp_server_info_t * tcp_server, connection_t * client)
{

    int i;

    assert(client);
    assert(tcp_server);

    // Информация по коннекте уже существует?
    for(i = 0; i < MAX_CONNECTIONS; i++)
    {
        // Обновить
        if(tcp_server->clients_info[i].client == client)
        {
            gettimeofday(&tcp_server->clients_info[i].last_update, 0);
            return;
        }
    }

    // Информации по коннекте не сущействует?
    for(i = 0; i < MAX_CONNECTIONS; i++)
    {

        // Создать запись на месте пустой
        if(tcp_server->clients_info[i].client == 0)
        {
            tcp_server->clients_info[i].client  = client;
            gettimeofday(&tcp_server->clients_info[i].last_update, 0);

            return;
        }

    }

}

/** @brief Функция для закрытия клиентсокго подключения
 * @param conn - указатель на закрываемый коннект
 * @return Возвращает 1 в случае успеха
 */
int tcp_server_close_client(connection_t * conn)
{

    int i;
    const common_info_t * info = 0;
    tcp_server_info_t * server_info = 0;

    assert(conn);

    info = get_common_info();
    server_info = info->server_info;

    assert(server_info);

    for(i = 0; i < MAX_CONNECTIONS; i++)
    {
        if(server_info->clients_info[i].client == conn)
        {
            server_info->clients_info[i].client = 0;
            break;
        }
    }

    if(server_info->nclients)
        server_info->nclients--;

    //printf("Number of clients: %d\n", info->tcp_server_info->nclients);
    va_list list;
    return high_client_close(conn, list);

}

/** @brief Функция для проверки подключений
 * @param tcp_server_info - указатель структуру с информацией о сервере
 * @param size - кол-во проверяемых клиентов
 * @param timeout_ms - лимит бездейсвия в миллисекундах
 */
void tcp_server_check_clients(tcp_server_info_t * tcp_server_info, int size, uint32_t timeout_ms)
{

    int i;

    double now_float = 0;
    struct timeval now = {0};

    assert(tcp_server_info);
    assert(size >= 0);

    gettimeofday(&now, 0);

    now_float = time_to_mks(now);

    for(i = 0; i < size; i++)
    {

        if(!tcp_server_info->clients_info[i].client)
            continue;

        if(now_float - time_to_mks(tcp_server_info->clients_info[i].last_update) >= 1000.0 * timeout_ms)
            connections_pool_invalidate_obj(tcp_server_info->clients_info[i].client->pool, tcp_server_info->clients_info[i].client);
    }

}
