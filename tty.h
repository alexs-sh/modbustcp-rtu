#ifndef TTY_H_INCLUDED
#define TTY_H_INCLUDED

#ifndef GLOBAL_H_INCLUDED
#include "global.h"
#endif  // GLOBAL_H_INCLUDED

#define TTY_INVALID -1 /**< Неверный дескриптор последовательного порта */
#define TTY_PARITY_NONE 0 /**< Нет проверки четности */
#define TTY_PARITY_EVEN 1 /**< Проверка четности - ЧЕТ*/
#define TTY_PARITY_ODD 2  /**< Проверка четности - НЕЧЕТ */

#define TTY_FLOW_NONE 0 /**< Управление потоком данных - отключено */
#define TTY_FLOW_HARDWARE 1 /**< Управление потоком данных - аппаратно */

#define TTY_NONBLOCK_MODE 0 /**< Режим блокировки - неблокируемый*/
#define TTY_BLOCK_MODE 1 /**< Режим блокировки - блокируемый */

#define TTY_VALIDATE(t) \
  ((t) > TTY_INVALID) /** Проверка корректности дескриптора посл. порта */

int tty_open(const char* name);
int tty_flush(int fd);
int tty_close(int fd);
int tty_setup(int fd, uint32_t speed, uint8_t parity, uint8_t data_bits,
              uint8_t flow, uint8_t block, uint8_t stop_bits);
int tty_get_input_data_len(int fd);
int tty_read(int fd, void* dst, uint16_t size);
int tty_write(int fd, const void* src, uint16_t size);
#endif  // TTY_H_INCLUDED

#ifdef UTILS_TEST
void tty_tests();
#endif
