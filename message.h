#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#ifndef GLOBAL_H_INCLUDED
#include "global.h"
#endif  // GLOBAL_H_INCLUDED

#ifndef connection_H_INCLUDED
#include "connection.h"
#endif

#define MB_HEADER_SIZE                                                                                                         \
  6 /**< Минимальный размер сообщения ModbusTCP (размер заголовка сообщения) \
     */

#define MAX_MESSAGE_SIZE 255 /**< Максимальный размер тела сообщения Modbus*/

#define MESSAGE_BUFFER_SIZE 256 + 32 /**< Размер буфера сообщения */

#define MB_ILLEGAL_DATA_ERROR 0x3 /**< Код ошибки Modbus. Неверный адрес */

#define MB_SLAVE_BUSY_ERRROR 0x6 /**< Код ошибки Modbus. устройство занято */

#define MB_NO_RESPOND_ERROR 0xB /**< Код ошибки Modbus. Нет ответа */

/**@struct mb_pdu_t
 * Структура для описания PDU ModbusTCP/RTU
 */
typedef struct {
  uint16_t id; /**< Номер запроса. Только ModbusTCP */

  uint16_t proto; /**< Тип протокола. Только ModbusTCP */

  uint16_t len; /**< Длина пакета с данными пакета. Только ModbusTCP */

  uint8_t data[MESSAGE_BUFFER_SIZE]; /**< Тело сообщения Modbus */

} mb_pdu_t;

/**@struct message_t
Структура для описания обрабатываемых сообщений сообщения
*/
typedef struct {
  connection_t*
      src_connection;  ///< Параметры подключения, получившего сообщение

  uint32_t src_ip;  ///<  IP - адрес отправителя

  uint16_t src_port;  ///< Номер порта отправителя

  uint8_t priority;  ///< Приоритет сообщения

  struct timeval timestamp;  ///< Метка времени

  mb_pdu_t pdu;  ///< Данные modbus

} message_t;

int message_make_error(message_t* message, uint8_t error);

void message_link_tcp_source(message_t* message, connection_t* conn);

void message_link_udp_source(message_t* message, connection_t* conn,
                             uint32_t ip_addr, uint16_t port);

void message_make_timestamp(message_t* message);

int message_validate_source(message_t* message);

int message_to_buffer(message_t* message, void* src_buffer, uint16_t src_size);

int message_clear(message_t* message);

#endif  // MESSAGE_H_INCLUDED
