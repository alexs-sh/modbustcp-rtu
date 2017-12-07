#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif // _SYS_TIME_H

#ifndef	_STRING_H
#include <string.h>
#endif // _STRING_H

#ifndef _ARPA_INET_H
#include <arpa/inet.h>
#endif // _ARPA_INET_H

#include "message.h"
#include "sockets.h"


/** @brief Функция для формирования сообщения об ошибке
 * @param message - указатель на заполняемое сообщения
 * @param errror - код ошибки modbus
 * @return Возвращает 1 в случае успеха иначе 0
 */
int message_make_error(message_t * message, uint8_t error)
{

    const int EXCEPTION_PDU_SIZE   = 3; /* размер pdu с сообщением*/
    const int EXCEPTION_INDEX = 2;      /* позиция байта с кодом сообщения */
    const int FUNC_INDEX = 1;           /* позиция байта с кодом функции */

    assert(message);

    /* Добавить информацию об ошибке */
    message->pdu.data[FUNC_INDEX] |= 0x80;
    message->pdu.data[EXCEPTION_INDEX] = error;
    message->pdu.len = EXCEPTION_PDU_SIZE;

    return 1;

}

/** @brief Функция для очистки сообщения
 * @param message - указатель на заполняемое сообщения
 * @return Возвращает 1 в случае успеха
 */
int message_clear(message_t * message)
{

    assert(message);
    memset(message, 0, sizeof(*message));
    return 1;
}


/** @brief Функция связывания сообщения с клиентом
 * @param message - указатель на заполняемое сообщения
 * @param conn - указатель на связываемый коннект
 */
void message_link_tcp_source(message_t * message, connection_t * conn) {

    assert(message);
    assert(conn);

    if(!SOCKET_VALIDATE(conn->fd))
        return;

    message->src_connection = conn;
    message->src_ip = socket_get_client_ip(conn->fd);
    message->src_port = socket_get_client_port(conn->fd);

}

/** @brief Функция связывания сообщения с клиентом
 * @param message - указатель на заполняемое сообщения
 * @param conn - указатель на связываемый коннект
 */
void message_link_udp_source(message_t * message, connection_t * conn, uint32_t ip_addr, uint16_t port) {

    assert(message);
    assert(conn);

    if(!SOCKET_VALIDATE(conn->fd))
        return;

    message->src_connection = conn;
    message->src_ip = ip_addr;
    message->src_port = port;
}

/** @brief Функция для добавления метки времени в сообщение
 * @param message - указатель на заполняемое сообщения
 */
void message_make_timestamp(message_t * message) {

    assert(message);
    gettimeofday(&message->timestamp, 0);

}

/** @brief Функция записи сообщения во внешний буфер
 * @param message - указатель на заполняемое сообщения
 * @param src_buffer - указатель на принимающий буфер
 * @param src_size - размер принимающего буфера
 */
int message_to_buffer(message_t * message, void * src_buffer, uint16_t src_size) {

    uint16_t * dst_ptr = 0;

    assert(message);
    assert(src_buffer);

    if(src_size < MB_HEADER_SIZE + message->pdu.len)
        return 0;

    dst_ptr = (uint16_t *)src_buffer;
    // записать заголовок сообщения
    *dst_ptr++ = htons(message->pdu.id);
    *dst_ptr++ = htons(message->pdu.proto);
    *dst_ptr++ = htons(message->pdu.len);

    //записать тело сообщения
    memcpy(dst_ptr, &message->pdu.data, message->pdu.len);

    return MB_HEADER_SIZE + message->pdu.len;
}


/** @brief Функция для проверки корректности дескриптора, связанного с сообщением
 * @param message - указатель на проверяемое сообщения
 * @return Возвращает 1 в случае успеха иначе 0
 */
int message_validate_source(message_t * message) {

    /* Порт и ip адрес дескриптора остались неизменными ?*/
    return  message->src_ip == socket_get_client_ip(message->src_connection->fd) &&
            message->src_port == socket_get_client_port(message->src_connection->fd);
}
