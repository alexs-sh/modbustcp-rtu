#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#ifndef _STDINT_H
#include <stdint.h>
#endif  // _STDINT_H

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef _ASSERT_H
#include <assert.h>
#endif  // _ASSERT_H

#define BYTE_SIZE 8  ///< пересчет мкс в мс

#define MAX_IP_ADDR 64  ///< макс. длина строки с ip - адресом / именем хоста

//#define MAX_DEVICES     256         ///< макс. кол-во устройств Modbus

#define MAX_QUEUE_SIZE 16  ///< размер очереди сообщений

#define MAX_CONNECTIONS 32  ///< макс. кол-во одновременных подключений

#define MAX_DEV_NAME 64  ///< макс. имя посл. порта

#define MS 1000  ///< пересчет мкс в мс

/* Установить бит с номером bit в массиве данных dst */
#define setbit(dst, bit) \
  (((uint8_t *)(dst))[(bit) / BYTE_SIZE] |= 1 << ((bit) % BYTE_SIZE))

/* Прочитать бит с номером bit в массиве данных dst */
#define getbit(dst, bit) \
  ((((uint8_t *)(dst))[(bit) / BYTE_SIZE] & (1 << ((bit) % BYTE_SIZE))) > 0)

#define INVALID_FD -1  ///< Неверный файловый дескриптор

#define VALIDATE_FD(f) ((f) != INVALID_FD)  ///< Проверка дескриптора

#define time_to_mks(t)      \
  ((t).tv_sec * 1000000.0 + \
   (t).tv_usec)  ///< Перевод времени из struct timveal в микросекунды

#define mks_to_time(dt)                                           \
  (struct timeval) {                                              \
    (long)(dt) / 1000000, (dt) - ((long)(dt) / 1000000) * 1000000 \
  }  ///< Перевод времени из struct timveal в микросекунды

typedef struct common_info_t common_info_t;

typedef struct settings_t settings_t;

/**@struct settings_t
 * @brief Структура для хранения основынх настроек программы
 */
struct settings_t {
  uint8_t daemon;  ///< работать в фоновом режиме

  uint8_t view_mode;  ///< режим просмотра

  uint8_t udp;  ///работа через UDP

  char ip[MAX_DEV_NAME];  ///< привязка к ip адресе

  uint16_t port;  ///< прослушиваемый порт

  char device[MAX_DEV_NAME];  ///< имя посл. устройства

  uint32_t speed;  ///< скорость посл. устройства

  uint8_t parity;  ///< четность посл. устройства

  uint8_t data_bits;  ///< биты данных посл. устройства

  uint8_t stop_bits;  ///< стоповые биты посл. устройства

  uint32_t
      silence_timeout_ms;  ///< таймаут на сброс сообщения на посл. устройстве

  uint32_t
      disconnect_timeout_ms;  ///< таймаут на отключение неактивных клиентов

  uint32_t lifetime_timeout_ms;  ///< время жизни сообщения в очереди

  // uint8_t queue_size;             ///< размер очереди сообщений

  uint8_t trace;  ///< вывод сообщений на экран

  uint8_t err_code_timeout;  ///< код ошибки передаваемый на ВУ в случае
                             ///< превышения времени ожидания ответа. По
                             ///< умолчанию 11.

  uint8_t err_code_overflow;  ///< код ошибки, выдваемый ри переполнении очереди
                              ///< сообщений. По умолчанию 11.

  uint8_t err_code_illegal;  ///< код ошибка, выдаваемый при получении
                             ///< некорректных данных по посл. порту. по
                             ///< умолчанию 3.

  uint16_t cmd_srv_port;  ///< Порт для сервера - управления

  uint8_t net_trace;  ///< Вывод сообщений через сеть
};

/**@struct common_info_t
 * @brief Структура для обмена данными между всеми мордулями прораммы
 */
struct common_info_t {
  struct connections_pool_t *connections_pool;  ///< пул подключений

  struct connection_t *std_in;  ///< указатель на стандартный ввод

  struct connection_t
      *cmd_srv;  ///< указатель на приемник управляющий подключений

  struct connection_t *cmd_client;  ///< указатель на коннект для управления

  struct settings_t *settings;  ///< указатель на структуру с настройками

  // struct tcp_server_info_t * tcp_server_info;             ///< указатель на
  // структуру с данными о сервере и клиентах

  struct low_client_info_t
      *tty_info;  ///< указатель на структуру с данными о посл. порте

  void *server_info;

  int work;
};

common_info_t *get_common_info();

void set_common_info(common_info_t *info);

#endif  // GLOBAL_H_INCLUDED
