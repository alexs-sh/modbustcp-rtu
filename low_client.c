#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif // _SYS_TIME_H


#ifndef	_UNISTD_H
#include <unistd.h>
#endif // _UNISTD_H


#ifndef	_STDLIB_H
#include <stdlib.h>
#endif // _STDLIB_H


#ifndef	_STRING_H
#include <string.h>
#endif // _STRING_H

#ifndef	_TIME_H
#include <time.h>
#endif // _TIME_H

#ifndef LOW_CLIENT_H_INCLUDED
#include "low_client.h"
#endif

#ifndef HIGH_CLIENT_H_INCLUDED
#include "high_client.h"
#endif

#ifndef CONNECTIONS_POOL_H_INCLUDED
#include "connections_pool.h"
#endif

#ifndef TTY_H_INCLUDED
#include "tty.h"
#endif

#ifndef TRACE_H_INCLUDED
#include "trace.h"
#endif

#define UNIT_ID_POS     0      /*< номер устройства - первый байт в сообщении */
#define FUNC_NUM_POS    1      /*< номер устройства - первый байт в сообщении */
#define BROADCAST_ID    255   /*< признак широковещательной рассылки */


static int low_client_read(connection_t * conn);
static int low_client_send_to_port(connection_t * conn);
static int low_client_close(connection_t * conn);
static int low_client_is_clear(low_client_info_t * client_info);
static void low_client_set_status(low_client_info_t * client_info, tty_status status);
static void low_client_update_info(low_client_info_t * client_info);

static uint16_t calc_crc16(const uint8_t * buf, uint16_t nbytes);

static const int16_t crc_16_tab[] =
{
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
    0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
    0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
    0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
    0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
    0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
    0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
    0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
    0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
    0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
    0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
    0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
    0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
    0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
    0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
    0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
    0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
    0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
    0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
    0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
    0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
    0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
    0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
    0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
    0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
    0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
    0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
    0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
    0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
    0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
    0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
    0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};


/** @brief Функция для  создания обработчика посл. порта
 * @return Возвращает 1 в слуае успеха.
 */
int low_client_create()
{

    connection_t * tty_conn = 0;

    common_info_t * info = get_common_info();

    assert(info->settings);

    // выдлеить память под струткуру с данными
    info->tty_info = calloc(1, sizeof(*info->tty_info));

    if(!info->tty_info)
        goto error;

    // найти свободный коннект для обработки сообщений
    tty_conn = connections_pool_get_free_obj(info->connections_pool);

    if(!tty_conn)
        goto error;

    // подключить порт
    tty_conn->fd = tty_open(info->settings->device);

    //printf("Open tty device %s with result %d\n", info->settings->device, tty_conn->fd);
    if(tty_conn->fd == INVALID_FD)
        goto error;
    // выполнить настройки порта
    if(!tty_setup(tty_conn->fd, info->settings->speed, info->settings->parity, info->settings->data_bits, TTY_FLOW_NONE, TTY_NONBLOCK_MODE, info->settings->stop_bits))
        goto error;

    //  связать файловый дескриптор с функциями - обработчиками
    tty_conn->read_func = (conn_handler_t)low_client_read;
    tty_conn->write_func = (conn_handler_t)low_client_write;//low_client_send_to_port;
    tty_conn->close_func = (conn_handler_t)low_client_close;
//    tty_conn->info = info;

    // обнулить очередь сообщений
    queue_init(&info->tty_info->input_queue);

    info->tty_info->tty = tty_conn;
    info->tty_info->rx = 0;
    info->tty_info->tx = 0;
    info->tty_info->valid_frames = 0;
    info->tty_info->invalid_frames = 0;
    info->tty_info->dropped_frames = 0;

    return 1;

error:
    if(info->tty_info)
        free(info->tty_info);

    if(tty_conn)
        connections_pool_release_obj(info->connections_pool, tty_conn);

    return 0;

}


/** @brief Функция для записи сообщения в очередь порта
 * @param conn - указатель на коннект посл. порта
 * @param args - список аргументов (указатель на message_t)
 * @return Возвращает 1 в слуае успеха.
 */
int low_client_write(connection_t * conn, va_list args)
{

    low_client_info_t * tty_info = 0;
    const common_info_t * info = 0;
    message_t * message = 0;

    assert(conn);
//    assert(conn->info);

    info = get_common_info(); //connection_get_info(conn);
    message = va_arg(args, message_t *);
    tty_info = info->tty_info;

    assert(info);
    assert(message);
    assert(tty_info);

    // есть место в очереди?
    if(queue_overflow(&tty_info->input_queue))
    {

        trace_out(0, ANSI_COLOR_RED, "OVERFLOW");

        if(message_validate_source(message))
        {
            if(info->settings->err_code_overflow) {
                message_make_error(message, info->settings->err_code_overflow);//NO_RESPOND_ERROR);
                connection_write(message->src_connection, message);
            }
        }

        return 0;
    }

    // добавть сообщение и вызвать обработчик
    queue_push(&tty_info->input_queue, message);

    low_client_send_to_port(tty_info->tty);

    return 1;

}

/** @brief Функция для проверки состояния посл. порта.
 * Если сообщение обрабатывается должшту заданного интервала, оно будет сброшено
 */
void low_client_check()
{

    common_info_t * info = get_common_info();
    struct timeval current_time;
    double now, last, limit;
    low_client_info_t * tty_info = 0;

    assert(info->tty_info);

    assert(info->tty_info->tty);

    tty_info = info->tty_info;
//    printf("\n-Low client check!-\n");
    // не применять к пустому порту
    if(low_client_is_clear(tty_info))
        return;


    // сформиовать метки времени
    gettimeofday(&current_time, 0);

    /* все таймауты рассчитываются в микросекундах */
    now = time_to_mks(current_time);
    last = time_to_mks(tty_info->last_update);
    limit = info->settings->silence_timeout_ms * MS;

    /* интервал истек ? */
    if(now - last > limit)
    {



        // сбросить порт
        low_client_set_status(tty_info, CLEAR);
        tty_flush(tty_info->tty->fd);

        // Вывести информацию и скинутом сообщении
        trace_out(0, ANSI_COLOR_RED, "DROP REQUEST");
        //trace_buffer_out(ANSI_COLOR_RED, tty_info->current_message.pdu.data, tty_info->current_message.pdu.len);

        // если код ошибки задан для данной ситуации, то уведомить клиента
        if(info->settings->err_code_timeout) {

            message_make_error(&tty_info->current_message, info->settings->err_code_timeout);
            connection_write(tty_info->current_message.src_connection, &tty_info->current_message);

        }

        // уведомить ВУ
        //void high_client_write(connection_t * conn, va_list args);
        //connection_write(tty_info->current_message.src_connection, &tty_info->current_message);

        /* перейти к сл. сообщению */
        low_client_send_to_port(tty_info->tty);

        tty_info->dropped_frames++;

    }
}

void low_client_invalidate_conn(low_client_info_t * tty, connection_t const *conn)
{

    assert(tty);
    assert(conn);

    queue_invalidate_recs(&tty->input_queue, conn);

    // Если активное сообщение связано с удаляемым коннектом,
    // то повреждаем дескриптор приемника, чтобы сообщение не могло быть
    // отпавлено назад. При этом продолжаем читать само
    // сообщение из порта. Дальнейшее чтение необходимо, чтобы не сбить
    // обмен.
    /*if(tty->current_message.src_connection == conn){
        tty->current_message.src_connection->fd = INVALID_FD;
        printf("Invalidate FD!\n");
    }*/

}
/** @brief Функция расчета контрольной суммы CRC - 16 по таблице
 *
 * @param buf - указатель на буфер - источник данных
 * @param nbytes - размер буфера в байтах
 * @return Возвращает контрольную сумму
 *
 */
static uint16_t calc_crc16(const uint8_t * buf, uint16_t nbytes)
{
    uint16_t crc = 0xffff;

    while (nbytes--)
        crc = (uint16_t)(crc >> 8) ^ crc_16_tab[(crc ^ *buf++) & 0xff];
    /* Переключение BigEndian - LittleEndian*/
#ifdef MX_BIG_ENDIAN
    crc = ((crc & 0x00FF) << 8) | ((crc & 0xFF00) >> 8);
#endif
    return crc;
}

/** @brief Функция для чтения данных, поступивших на посл. порт
 * @param conn - указатель на обрабатываемый коннект
 * @return В случае успеха возвращает 1.
 */
static int low_client_read(connection_t * conn)
{


//    printf("Low client!!\n");
    int result = 0;

    uint8_t * ptr = 0;

    const common_info_t * info = 0;

    low_client_info_t * tty_info = 0;

    message_t * current_message = 0;

    struct timespec wait = {0, 5 * 1000 * 1000};

    assert(conn);
//    assert(conn->info);

    info = get_common_info();//connection_get_info(conn);
    tty_info = info->tty_info;

    if(low_client_is_clear(tty_info))
    {
        tty_flush(tty_info->tty->fd);
        //printf("Get spam!\n");
        return 0;
    }

    // ждать 5 миллисекнд.
    // 5 мс т.к. меньший интервал приведет к более частому вызову функции,
    // а больший, к увеличению времени на чтени одного сообщения.
    nanosleep(&wait, 0);

    current_message = &tty_info->current_message;

    ptr = current_message->pdu.data + current_message->pdu.len;

    // прочитать данные из порта и добавить в хвост буфера сообщения
    result = tty_read(tty_info->tty->fd, ptr, MESSAGE_BUFFER_SIZE - current_message->pdu.len);

    if(result <= 0)
        return 0;

    tty_info->rx += result;

    low_client_update_info(tty_info);
    current_message->pdu.len += result;

    // проверить коррекнтоность сообщения
    if(current_message->pdu.len > MAX_MESSAGE_SIZE ||
            tty_info->current_slave != current_message->pdu.data[UNIT_ID_POS])
    {
        // вслучае ошибкт даннх восстановить номер устройства и код функции,
        // для корректного формирования ответного сообщения на ВУ
        current_message->pdu.data[UNIT_ID_POS] = tty_info->current_slave;
        current_message->pdu.data[FUNC_NUM_POS] = tty_info->current_func;

        trace_out(0, ANSI_COLOR_RED, "READ INVALID FRAME");
        trace_buffer_out(ANSI_COLOR_RED, current_message->pdu.data, current_message->pdu.len);

        // сформировать сообщение об ошибке и передать на ВУ
        if(info->settings->err_code_illegal)
        {
            message_make_error(current_message, info->settings->err_code_illegal);
            // уведомить ВУ
            //void high_client_write(connection_t * conn, va_list args);
            connection_write(current_message->src_connection, current_message);
    //        high_client_process_message(current_message);
        }


        // подготовить порт к обработке сл. сообщения
        low_client_set_status(info->tty_info, CLEAR);
        tty_flush(tty_info->tty->fd);

/* TODO (alexs#1#): Сделать таймаут, задаваемый через параметры ! */        wait = (struct timespec){0, 5 * 1000 * 1000};
        nanosleep(&wait, 0);

        low_client_send_to_port(info->tty_info->tty);

        tty_info->invalid_frames++;


        return 0;
    }

    // контрольřеая сумма совпала?
    if(calc_crc16(current_message->pdu.data, current_message->pdu.len) == 0)
    {

        // извечь 2 байта контрольной суммы
        current_message->pdu.len -= 2;

        // отправить сообщение на ВУ
        //void high_client_write(connection_t * conn, va_list args);
        connection_write(current_message->src_connection, current_message);

        trace_out(0, ANSI_COLOR_YELLOW, "READ");
        trace_buffer_out(ANSI_COLOR_YELLOW, current_message->pdu.data, current_message->pdu.len + 2);

        //подготовить порт к обработке сл. сообщения
        low_client_set_status(info->tty_info, CLEAR);

/* TODO (alexs#1#): Сделать таймаут, задаваемый через параметры ! */        wait = (struct timespec){0, 5 * 1000 * 1000};
        nanosleep(&wait, 0);

        low_client_send_to_port(info->tty_info->tty);

        tty_info->valid_frames++;

        /*int i;
        printf("Get from tty:\n");
        for(i = 0; i < current_message->pdu.len; i++)
            printf("%d ", current_message->pdu.data[i]);
        printf("\n");*/



        return 1;
    }
    else
    {
        // продолжить чтение и накопления сообщения
        low_client_set_status(info->tty_info, READ_ANSWER);
    }

    return 0;
}

/** @brief Функция для записи данных в посл. порт
 * @param conn - указатель на обрабатываемый коннект
 * @return В случае успеха возвращает 1.
 */
static int low_client_send_to_port(connection_t * conn)
{

    const common_info_t * info = 0;
    low_client_info_t * tty_info = 0;
    message_t * current_message = 0;

    struct timeval current_time;
    double now, last, limit;

    int have_message = 0;
    int result = 0;

    uint8_t * ptr = 0;
    uint16_t crc = 0;

    struct timespec wait = {0, 10 * 1000 * 1000};

    assert(conn);

//    printf("Write to TTY!\n");

    info = get_common_info();//connection_get_info(conn);
    tty_info = info->tty_info;


    // в занятый пот новое сообщение не запишешь
    if(!low_client_is_clear(tty_info)) {
    //    trace_out(ANSI_COLOR_BLUE, "PORT BUSY\n");
        return 0;
    }

    // сформировать метки времени
    gettimeofday(&current_time, 0);
    now = time_to_mks(current_time);
    limit = info->settings->lifetime_timeout_ms * MS; /* в микросекундах */

    current_message = &tty_info->current_message;

    // выбрать из очереди подходящее сообщение
    while(queue_pop(&tty_info->input_queue, current_message))
    {

        // Не обрабатывать сообщения, у которых коннект назначения обнулен.
        // Это признак того, что подключене уже закрыто.
        if(!current_message->src_connection)
            continue;

        // Если контроль времени жизни сообщения отключен, то любое сообщение из очереди
        // счиатается корректным и обрабатывается.
        if(info->settings->lifetime_timeout_ms == 0)
        {
            have_message = 1;
            break;
        }

        // Иначе выполняется проверка на устаревание сообщения
        last = time_to_mks(current_message->timestamp);

        // сообщение не устарело?
        if(now - last < limit)
        {
            // обработать
            have_message = 1;
            break;
        }
        else
        {

            //message_make_error(current_message, NO_RESPOND_ERROR);
            /* TODO (alexs#9#): Проверить есть ли необходимость уведомлять клиентов об устаревании сообщений в очереди */
            // вывод данных об отброшенном сообщении
            trace_out(0, ANSI_COLOR_RED, "REMOVE OLD MESSAGE");
            trace_buffer_out(ANSI_COLOR_RED, current_message->pdu.data, current_message->pdu.len);

            // уведомить ВУ
            //void high_client_write(connection_t * conn, va_list args);
            //connection_write(current_message->src_connection, current_message);

        }
    }

    // подходящего сообщения так и не найдено. Вернуться.
    if(!have_message)
        return 0;

    // сформировать сообщение для записи в посл. порт
    ptr = current_message->pdu.data;

    /* рассчитать контрольную сумму */
    crc = calc_crc16(ptr, current_message->pdu.len);

    /* добавить 2 байта контрольной суммы в конец сообщения*/
    memcpy(ptr + current_message->pdu.len, &crc, sizeof(crc));

    result = tty_write(info->tty_info->tty->fd, ptr, current_message->pdu.len + sizeof(crc));

    if(result <= 0)
    {

        trace_out(1, ANSI_COLOR_RED, "SEND ERROR");
        return 0;

    }
    else
    {
        trace_out(0, ANSI_COLOR_GREEN, "SEND");
        trace_buffer_out(ANSI_COLOR_GREEN, current_message->pdu.data, current_message->pdu.len + sizeof(crc));
    }


    tty_info->tx += result;

    // при обработке не широковещательных сообщений необходимо дождаться ответа
    if(current_message->pdu.data[UNIT_ID_POS] != BROADCAST_ID)
    {

        tty_info->current_slave = current_message->pdu.data[UNIT_ID_POS];
        tty_info->current_func = current_message->pdu.data[FUNC_NUM_POS];

        low_client_set_status(tty_info, WAIT_ANSWER);
        low_client_update_info(tty_info);

    }
    else
    {
        // после отправки широковещательного сообщения переходим к сл. сообщению
        nanosleep(&wait, 0);
        low_client_send_to_port(conn);
    }

    // подготовить к приему
    current_message->pdu.len = 0;

    return 1;
}


/** @brief Функция для закрытия посл. порта
 * @param conn - указатель на обрабатываемый коннект
 * @return В случае успеха возвращает 1.
 */
static int low_client_close(connection_t * conn)
{

    const common_info_t * info = 0;

    assert(conn);
//    assert(conn->info);

    info = get_common_info();//connection_get_info(conn);

    tty_close(conn->fd);

    if(info->tty_info)
        free(info->tty_info);

//    printf("Close low-client!\n");

    return 1;
}


/** @brief Функция для  обновления данных о опрте
 * @param client_info - указатель данные порта
 */
static void low_client_update_info(low_client_info_t * client_info)
{

    assert(client_info);
    gettimeofday(&client_info->last_update, 0);
//    printf("TTY Client update!\n");

}



/** @brief Функция для определения состояние порта
 * @param client_info - указатель на данные о порта
 * @return Возвращает 1 если порт занят.
 */
static int low_client_is_clear(low_client_info_t * client_info)
{

    assert(client_info);
    return client_info->status == CLEAR;
}

/** @brief Функция для задания состояния порта
 * @param client_info - указатель на данные о порта
 * @param status - состояние порта
 *          Допустимые значения: \n
 *           CLEAR - нет активной задачи
 *           WAIT_ANSWER - идет ожидание ответа
 *           READ_ANSWER - идет считываение ответа
 */
static void low_client_set_status(low_client_info_t * client_info, tty_status status)
{

    assert(client_info);
    client_info->status = status;
}

