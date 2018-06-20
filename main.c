#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "cmd_server.h"
#include "connection.h"
#include "connections_pool.h"
#include "high_client.h"
#include "low_client.h"
#include "message.h"
#include "sockets.h"
#include "std_input.h"
#include "tcp_server.h"
#include "trace.h"
#include "tty.h"
#include "udp_server.h"

/* Прототипы вспомогательных фукнций */
static void print_help();
static int parse_settings(settings_t *settings, int argc, char *argv[]);
static int init_common_info(common_info_t *info);
static void release_common_info(common_info_t *info);

/* Настройки по умолчанию */
settings_t current_settings = {0, 0, 0, "0.0.0.0", 502, "/dev/null",
                               //"/dev/ttyUSB0",
                               //"/dev/random",
                               //"/dev/ttyM0",
                               38400, TTY_PARITY_ODD, 8, 1, 50, 30000, 1000, 0,
                               MB_NO_RESPOND_ERROR, MB_SLAVE_BUSY_ERRROR,
                               MB_ILLEGAL_DATA_ERROR, 0, 0};

/** @brief Функция для вывода справки по основным ключам запуска
 */
void print_help() {
  printf(
      "Modbus TCP / RTU driver\n"

      "\t-b                Run driver in background mode. Disabled for uLinux "
      "devices (UC - 7110)\n"
      "\t-v                View-mode. Enable input and output. Support "
      "commands 'quit' and 'info'\n"
      "\t-U                UDP.\n"
      "\t-I <name>         TCP server. Linked IP address (\"0.0.0.0\" by "
      "default)\n"
      "\t-P <value>        TCP server. Port number (502 by default)\n"
      "\t-D <name>         Seral port. Name (\"/dev/null\" by default)\n"
      "\t-s <value>        Seral port. Speed (38400 by default)\n"
      "\t-p <value>        Seral port. Parity (ODD by default)\n"
      "\t-l <value>        Seral port. Data bits (8 by default)\n"
      "\t-e <value>        Seral port. Stop bits (1 by default)\n"
      "\t-t <value>        Seral port. Message drop timeout (50 ms by "
      "default)\n"
      "\t-d <value>        TCP server. Client disconnect timeout (30 000 ms by "
      "default)\n"
      "\t-w <value>        Queue. Message lifetime (1000 ms by default)\n"
      "\t-u <value>        Message.Error code for unresponsive device (11 by "
      "default)\n"
      "\t-o <value>        Message.Error code for queue overflow (6 by "
      "default)\n"
      "\t-i <value>        Message.Error code for illegal message data (3 by "
      "default)\n"
      "\t-c                CMD Server. Port for command server\n"
      "\t-h                help\n");
}

/** @brief Функция для разбора входных параметров
 *  @param settings - указатель на заполняемую структуру с настройками
 *  @param argc - кол-во параметров, переданных через командную строку
 *  @param argv - массив указателей на параметры
 *  @return Возвращает 1 в случае успешного завершения. Иначе 0.
 */
int parse_settings(settings_t *settings, int argc, char *argv[]) {
  /*Строка контролируемых параметров */
  const char *opt_string = "bvUI:P:D:s:p:l:e:t:d:w:q:u:o:i:c:h";
  int c;
  long tmp;

  if (argc <= 1) {
    print_help();
    return 0;
  }

  assert(settings);
  /* Разбор аргументов */
  while ((c = getopt(argc, argv, opt_string)) != -1) {
    switch (c) {
        /* фоновый режим*/
      case 'b':
        settings->daemon = 1;
        break;
        /* режим прсмотра */
      case 'v':
        if (settings->daemon)
          printf("View mode disabled by Background mode!\n");
        else
          settings->view_mode = 1;
        break;
        /* режим прсмотра */
      case 'U':
        settings->udp = 1;
        break;
        /* связанный ip-адрес*/
      case 'I':
        strncpy(settings->ip, optarg, MAX_DEV_NAME);
        break;
        /* номер порта */
      case 'P':
        tmp = atol(optarg);
        if (tmp > 0 && tmp < 0xFFFF)
          settings->port = tmp;
        else
          printf("Wrong port number (0...65535) \n");
        break;
        /* имя посл. устройства */
      case 'D':
        strncpy(settings->device, optarg, MAX_DEV_NAME);
        break;
        /* скорость обмена по посл. устройству.*/
      case 's':
        tmp = atol(optarg);
        if (tmp > 0)
          settings->speed = tmp;
        else
          printf("Wrong speed (1200, 2400, 4800, 9600, 19200, 38400...)\n");
        break;
        /* четость на посл. устройстве*/
      case 'p':
        if (strncasecmp("none", optarg, 5) == 0)
          settings->parity = TTY_PARITY_NONE;
        else if (strncasecmp("even", optarg, 5) == 0)
          settings->parity = TTY_PARITY_EVEN;
        else if (strncasecmp("odd", optarg, 4) == 0)
          settings->parity = TTY_PARITY_ODD;
        else
          printf("Wrong parity (none, even, odd)\n");
        break;
        /* кол-во бит данных */
      case 'l':
        tmp = atol(optarg);
        if (tmp > 0)
          settings->data_bits = tmp;
        else
          printf("Wrong data bits\n");
        break;
        /* кол-во стоп-бит */
      case 'e':
        tmp = atol(optarg);
        if (tmp > 0)
          settings->stop_bits = tmp;
        else
          printf("Wrong stop bits\n");
        break;
        /* таймаут на сброс сообщения с посл. порта */
      case 't':
        tmp = atol(optarg);
        if (tmp > 0)
          settings->silence_timeout_ms = tmp;  // * MS;
        else
          printf("Wrong drop timeout (> 0)\n");
        break;
        /* таймаут на отключение пассивных клиентов */
      case 'd':
        tmp = atol(optarg);

        if (tmp > 0)
          settings->disconnect_timeout_ms = tmp;  // * MS;
        else
          printf("Wrong disconnect timeout (> 0) \n");
        break;
        /* время жизни сообщения в очереди */
      case 'w':
        tmp = atol(optarg);
        if (tmp >= 0)
          settings->lifetime_timeout_ms = tmp;  //; * MS;
        else
          printf("Wrong message lifetime (>= 0) \n");
        break;
        /* размер очереди сообщений */
        /*case 'q':
            tmp = atol(optarg);
            if(tmp > 0)
                settings->queue_size = tmp;
            else
                printf("Wrong queue size \n");
            break;*/
        /* вывод справки */
        /* "\t-r <value>        Message.Error code for unresponsive device (11
by default)\n"
"\t-o <value>        Message.Error code for queue overflow (6 by default)\n"
"\t-i <value>        Message.Error code for illegal message data (3 by
default)\n"*/

      // код ошибки для неотвечающего устройства
      case 'u':
        tmp = atoi(optarg);
        if (tmp >= 0 && tmp <= 255)
          settings->err_code_timeout = tmp;
        else
          printf("Wrong unresponsive device code (0...255)\n");
        break;

      // код ошибки при переполнении очереди
      case 'o':
        tmp = atoi(optarg);
        if (tmp >= 0 && tmp <= 255)
          settings->err_code_overflow = tmp;
        else
          printf("Wrong overflow code (0...255)\n");

        break;

      // код ошибки при чтении некорректных данных из порта
      case 'i':
        tmp = atoi(optarg);
        if (tmp >= 0 && tmp <= 255)
          settings->err_code_illegal = tmp;
        else
          printf("Wrong illegal data code (0...255)\n");

        break;

      case 'c':
        tmp = atoi(optarg);
        if (tmp >= 0 && tmp <= 65535)
          settings->cmd_srv_port = tmp;
        else
          printf("Wrong command server port(0...65535)\n");
        break;

      case 'h':
        print_help();
        break;
      default:
        break;
    }
  }
  return 1;
}

/** @brief Функция инициализации основной инф. стурктуры
 *
 * @param info - указатель на общую информационную структуру
 */
static int init_common_info(common_info_t *info) {
  assert(info);

  memset(info, 0, sizeof(*info));

  info->settings = &current_settings;

  info->connections_pool =
      connections_pool_create(MAX_CONNECTIONS + 5, sizeof(void *));

  if (!info->connections_pool) return 0;

  cmd_server_create(info);

  info->work = 1;

  set_common_info(info);

  return 1;
}

/** @brief Функция уничтожения основной инф. стурктуры
 *
 * @param info - указатель на общую информационную структуру
 */
static void release_common_info(common_info_t *info) {
  assert(info);
  connections_pool_free(info->connections_pool);
}

static void check_common_info(common_info_t *info) {
  assert(info);

  // Проверка корректности таймаутов
  switch (info->settings->speed) {
    case 9600:
      if (info->settings->silence_timeout_ms < 150)
        trace_out(1, ANSI_COLOR_YELLOW,
                  "\tWARNING: Recommended timeout for speed 9600 >= 150 ms!");
      // printf("\tWARNING: Recommended timeout for speed 9600 >= 150 ms!\n");
      break;

    case 19200:
      if (info->settings->silence_timeout_ms < 100)
        trace_out(1, ANSI_COLOR_YELLOW,
                  "\tWARNING: Recommended timeout for speed 19200 >= 100 ms!");

      // printf("\tWARNING: Recommended timeout for speed 19200 >= 100 ms!\n");
      break;

    case 38400:
      if (info->settings->silence_timeout_ms < 50)
        trace_out(1, ANSI_COLOR_YELLOW,
                  "\tWARNING: Recommended timeout for speed 38400 >= 50 ms!");

      // printf("\tWARNING: Recommended timeout for speed 38400 >= 50 ms!\n");
      break;
    default:
      break;
  }

  // Провека таймаутов. Если контроль включен, то
  // время жизни сообщения должно быть значительно больше
  // таймаута, иначе  многие сообщения из очереди просто будут отброшены.
  /*if(info->settings->lifetime_timeout_ms &&
     (info->settings->lifetime_timeout_ms <
      info->settings->silence_timeout_ms * 2)) {
      trace_out(1, ANSI_COLOR_YELLOW,
                "\tWARNING: Silence timeout must be <= lifetime timeout * 2!");

      //printf("\tWARNING: Silence timeout must be <= lifetime timeout * 2!\n");
  }*/
}

/** @brief main и в Африке main
 */
int main(int argc, char *argv[]) {
  common_info_t info;

  const int CHECK_CLIENTS_MS = 1000;
  const int CHECK_SERIAL_MS = 20;
  const int POLL_TIMEOUT_MS = 10;

  float check_client_timeout = CHECK_CLIENTS_MS;
  float check_serial_timeout = CHECK_SERIAL_MS;

  trace_out(1, ANSI_COLOR_RESET, "Modbus Driver build %s %s\n", __DATE__,
            __TIME__);
  trace_out(1, ANSI_COLOR_RESET, "Start driver...\n");

  // прочитать настроки из аргементов
  if (!parse_settings(&current_settings, argc, argv)) return 0;

  // инициализировать общую информационную структуру данных
  if (!init_common_info(&info)) {
    trace_out(1, ANSI_COLOR_RED, "init_common_info failed!");
    goto finish;
  }

  check_common_info(&info);

  // создать сервер
  if (info.settings->udp) {
    if (!udp_server_create()) {
      trace_out(1, ANSI_COLOR_RED, "udp_server_create failed!");
      goto finish;
    }
  } else {
    if (!tcp_server_create()) {
      trace_out(1, ANSI_COLOR_RED, "tcp_server_create failed!");
      goto finish;
    }
  }

  // создать обработчик посл. порта
  if (!low_client_create()) {
    trace_out(1, ANSI_COLOR_RED, "low_client_create failed!");
    goto finish;
  }

  // в режме просмотра добавить обр. ст. ввода
  if (info.settings->view_mode) {
    stdin_create(&info);
  }

#ifndef MX_NO_DAEMON
  if (info.settings->daemon) daemon(0, 0);
#endif

  check_client_timeout = CHECK_CLIENTS_MS;
  check_serial_timeout = CHECK_SERIAL_MS;

  // поехали
  while (info.work) {
    // проверить и обработать активные соединения
    uint32_t result =
        connections_pool_process(info.connections_pool, POLL_TIMEOUT_MS);

    check_client_timeout -= result > 0 ? result : 1;
    check_serial_timeout -= result > 0 ? result : 1;

    // таймаут на проверку клиентов истек?
    if (!info.settings->udp && check_client_timeout < 0) {
      check_client_timeout = CHECK_CLIENTS_MS;
      tcp_server_check_clients(info.server_info, MAX_CONNECTIONS,
                               info.settings->disconnect_timeout_ms);
    }

    // таймаут на проверку посл. порта истек?
    if (check_serial_timeout < 0) {
      check_serial_timeout = CHECK_SERIAL_MS;
      low_client_check();
    }
  }

finish:
  // прибрать за собой
  release_common_info(&info);
  return 0;
}
