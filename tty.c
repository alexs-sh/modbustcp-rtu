#ifndef _TERMIOS_H
#include <termios.h>
#endif

#ifndef _FCNTL_H
#include <fcntl.h>
#endif

#ifndef _ASSERT_H
#include <assert.h>
#endif

#ifndef _UNISTD_H
#include <unistd.h>
#endif

#ifndef _SYS_IOCTL_H
#include <sys/ioctl.h>
#endif  // _SYS_IOCTL_H

#include "tty.h"

/** @brief Перевод скорости бит/с в константу, поятную функциям рабоыт с портом
 *
 * @param speed - скорость (бит/с).
 * @return Возвращает константу, соответсвующую скорости. Либо константу, соотв.
 * скорости 9600 Бит/с
 *
 */

static unsigned long tty_get_speed_code(unsigned long speed) {
  switch (speed) {
    case 50:
      return B50;
    case 75:
      return B75;
    case 110:
      return B110;
    case 134:
      return B134;
    case 150:
      return B150;
    case 200:
      return B200;
    case 300:
      return B300;
    case 600:
      return B600;
    case 1200:
      return B1200;
    case 1800:
      return B1800;
    case 2400:
      return B2400;
    case 4800:
      return B4800;
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 38400:
      return B38400;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    case 230400:
      return B230400;
    case 460800:
      return B460800;
    case 500000:
      return B500000;
    case 576000:
      return B576000;
    case 921600:
      return B921600;
    default:
      return B9600;
  }
}

/** @brief Функция открытия последовательного порта
 *
 * @param name - имя порта.
 * @return Возвращает дескриптор порта либо INVALID_FD
 *
 */
int tty_open(const char* name) {
  int fd = 0;
  assert(name);
  fd = open(name, O_RDWR | O_NOCTTY | O_NDELAY);
  return fd;
}

/** @brief Функция закрытия последовательного порта
 *
 * @param fd - дескриптор порта
 * @return Возвращает 1 в случае успешного закрытия порта
 *
 */
int tty_close(int fd) {
  assert(fd);

  if (!TTY_VALIDATE(fd)) return 0;

  close(fd);

  return 1;
}

/** @brief Функция настройки параметров последовательного порта
 *
 * @param fd - декскриптор порта
 * @param speed - скорость последовательного порта
 * @param parity - контроль четности. \I
 *  Допустимые значеения: \n
 *      TTY_PARITY_NONE - без контроля  \n
 *      TTY_PARITY_EVEN - четные        \n
 *      TTY_PARITY_ODD  - нечетные      \n
 * @param data_bits - сло бит данных. От 5 до 8.
 * @param flow - контроль потока        \n
        TTY_FLOW_NONE - без контроля
        TTY_FLOW_HARDWARE - аппаратный контроль
 * @param block - режим блокировки      \n
 *      TTY_NONBLOCK_MODE - неблокиующий \n
 *      TTY_BLOCK_MODE    - блокирующий  \n
 * @param stop_bits - кол - во стоповых бит. 1 или 2.
 * @return Возвращает 1 в случае успешного выполнения
 *
 */
int tty_setup(int fd, uint32_t speed, uint8_t parity, uint8_t data_bits,
              uint8_t flow, uint8_t block, uint8_t stop_bits) {
  struct termios settings;
  int flags = 0;

  /* Проверить дескриптор*/
  if (!TTY_VALIDATE(fd)) return 0;

  tcflush(fd, TCIOFLUSH);

  if (tcgetattr(fd, &settings) != 0) return 0;

  /* Обнулить настройки */
  settings.c_cflag &= ~(CS5 | CS6 | CS7 | CS8);
  settings.c_iflag &= !(IXON | IXOFF | IXANY);

  /* Настроить четности */
  switch (parity) {
    case TTY_PARITY_NONE:
      settings.c_cflag &= ~PARENB;
      settings.c_cflag &= ~PARODD;
      break;
    case TTY_PARITY_EVEN:
      settings.c_cflag |= PARENB;
      settings.c_cflag &= ~PARODD;
      break;
    case TTY_PARITY_ODD:
      settings.c_cflag |= PARENB;
      settings.c_cflag |= PARODD;
      break;
    default:
      settings.c_cflag &= ~PARENB;
      settings.c_cflag &= ~PARODD;
  }

  /* Настроить биты данных */
  switch (data_bits) {
    case 5:
      settings.c_cflag |= CS5;
      break;
    case 6:
      settings.c_cflag |= CS6;
      break;
    case 7:
      settings.c_cflag |= CS7;
      break;
    case 8:
      settings.c_cflag |= CS8;
      break;
    default:
      settings.c_cflag |= CS8;
  }

  /* Настроить управления потоком */
  if (flow == TTY_FLOW_HARDWARE)
    settings.c_cflag |= CRTSCTS;
  else
    settings.c_cflag &= ~CRTSCTS;

  settings.c_cflag |= CREAD | CLOCAL;
  settings.c_oflag = 0;
  settings.c_lflag = 0;
  settings.c_iflag = 0;
  settings.c_cc[VMIN] = 0;
  settings.c_cc[VTIME] = 10;

  /* Настроить стоповые биты*/
  switch (stop_bits) {
    case 1:
      settings.c_cflag &= ~CSTOPB;
      break;
    case 2:
      settings.c_cflag |= CSTOPB;
      break;
    default:
      settings.c_cflag &= ~CSTOPB;
  }

  /* Настроить скорость */
  cfsetispeed(&settings, tty_get_speed_code(speed));
  cfsetospeed(&settings, tty_get_speed_code(speed));

  /* Применить настройки порта */
  if (tcsetattr(fd, TCSANOW, &settings) != 0) return 0;

  /* Настройка режима блокировки */
  flags = fcntl(fd, F_GETFL);
  switch (block) {
    case TTY_BLOCK_MODE:
      flags &= (~O_NONBLOCK);
      break;
    case TTY_NONBLOCK_MODE:
      flags |= O_NONBLOCK;
      break;
    default:
      flags |= O_NONBLOCK;
      break;
  }
  /* Применить настройки блокировки */
  return fcntl(fd, F_SETFL, flags) == 0;
}

/** @brief Функция чтения из порта
 *
 * @param fd - декскриптор порта
 * @param src - указатель на буфер приемник
 * @param size - размер буфера - приемника
 * @return Возвращает результат функции read
 *
 */
int tty_read(int fd, void* dst, uint16_t size) {
  if (!TTY_VALIDATE(fd)) return 0;

  return read(fd, dst, size);
}

/** @brief Функция записи в порт
 *
 * @param fd - декскриптор порта
 * @param src - указатель на буфер источник
 * @param size - размер буфера - источника
 * @return Возвращает результат функции write
 *
 */
int tty_write(int fd, const void* src, uint16_t size) {
  if (!TTY_VALIDATE(fd)) return 0;

  return write(fd, src, size);
}

/** @brief Функция сброса данных
 * Функция очищает передающий и принимающий буфер устройства
 * @param fd - декскриптор порта
 * @return Возвращает 1 в случае успеха
 *
 */
int tty_flush(int fd) {
  if (!TTY_VALIDATE(fd)) return 0;
  tcflush(fd, TCIOFLUSH);
  return 1;
}

/** @brief Функция проверки наличия данный во входном буфере порта
 * @param fd - декскриптор порта
 * @return Возвращает кол-во байт данных во входном буфере порта
 *
 */
int tty_get_input_data_len(int fd) {
  int bytes = 0;

  if (!TTY_VALIDATE(fd)) return 0;

  ioctl(fd, FIONREAD, &bytes);
  return bytes;
}

/* */

#ifdef UTILS_TEST
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

void tty_tests() {
  errno = 0;
  fd_set set;
  struct timeval timeout = {2, 0};
  int i, port = tty_open("/dev/ttyUSB0");
  FD_ZERO(&set);

  if (!tty_setup(port, 38400, TTY_PARITY_ODD, 8, TTY_FLOW_NONE,
                 TTY_NONBLOCK_MODE, 1)) {
    printf("Couldn't setup port. Err =%d!\n", errno);
    return;
  }

  unsigned char buffer[] = {0x1, 0x3, 0x0, 0x2, 0x0, 0x4, 0xE5, 0xC9};
  unsigned char in_buffer[255];
  int result = 0;

  // for(i = 0; i < 100; i++) {

  result += write(port, buffer, sizeof(buffer));
  FD_SET(port, &set);

  switch (select(port + 1, &set, 0, 0, &timeout)) {
    case -1:
      printf("Error!\n");
      break;
    case 0:
      printf("Timeout!\n");
      break;
    default:
      sleep(1);
      result = read(port, in_buffer, 255);
      printf("Get %d bytes, err = %d\n", result, errno);
      for (i = 0; i < result; i++) printf("%d ", in_buffer[i]);
  }

  tty_close(port);
}

#endif
