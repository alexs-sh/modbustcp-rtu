#ifndef _STDLIB_H
#include <stdlib.h>
#endif  // _STDLIB_H

#ifndef _STRING_H
#include <string.h>
#endif  // _STRING_H

#include "input_queue.h"

/*queue_t * queue_create() {

    queue_t * queue = calloc(1, sizeof(* queue));
    return queue;
}*/

/** @brief Функция начальной инициализации очереди
 * @param queue - указатель очередь
 * @return В случае успеха возвращает 1.
 */
int queue_init(queue_t *queue) {
  int i;

  assert(queue);

  // Заполнить начальными значениями
  queue->read_pos = queue->write_pos = queue->size = 0;
  for (i = 0; i < MAX_QUEUE_SIZE; i++)
    memset(&queue->data[i], 0, sizeof(queue->data[0]));

  return 1;
}

/** @brief Функция для добавления элемента в конец очереди
 * @param queue - указатель на очередь
 * @param src - указатель на элемент
 * @return Возвращает 1 в случае успеха иначе 0.
 */
int queue_push(queue_t *queue, const void *src) {
  message_t const *new_msg = (message_t *)src;
  message_t *cur_msg = 0;
  int i;

  assert(queue);
  /* есть место для записи */
  // добавлена возможность пероезаписи элементов
  /*if(queue->size >= queue->max_size)
      return 0;*/

  // Выполнить проверку на дублирование сообщений. В один момент
  // времени в очереди не может существовать нескольких сообщений
  // от одного источника к одному устройству.
  // если такое случится, то необходимо обновить существующее сообщ. новым.
  for (i = queue->read_pos; i != queue->write_pos;
       i = (i + 1) % MAX_QUEUE_SIZE) {
    cur_msg = &queue->data[i];

    // Для замены сообщения источник данных и адрес устройства должны совпадать.
    // Адрес устройства - 0 байт в массиве сообщения
    if (cur_msg->src_connection == new_msg->src_connection &&
        cur_msg->pdu.data[0] == new_msg->pdu.data[0]) {
      // обновить сообщение
      memcpy(cur_msg, new_msg, sizeof(message_t));
      return 1;
    }
  }

  // Если мы здесь, то добавляем сообщения в конец очереди
  /* Перенести данные в буфер*/
  memcpy(&queue->data[queue->write_pos], src, sizeof(message_t));

  /* Сдвинуть указатель записи */
  queue->write_pos = (queue->write_pos + 1) % MAX_QUEUE_SIZE;
  if (queue->size < MAX_QUEUE_SIZE) queue->size = queue->size + 1;

  return 1;
}

/** @brief Функция для извлечения элемента из головы очереди
 * @param queue - указатель на очередь
 * @param src - указатель на элемент
 * @return Возвращает 1 в случае успеха иначе 0.
 */
int queue_pop(queue_t *queue, void *dst) {
  assert(queue);

  if (queue->size <= 0) return 0;
  /* Вычислить адрес памяти, куда будет добавлен объект
  адрес = начало буфера + номер записи * размер элемента */
  // uint8_t * ptr = queue->data + queue->read_pos * queue->element_size;

  /* Перенести данные и уменшить размер очереди */
  memcpy(dst, &queue->data[queue->read_pos], sizeof(message_t));
  queue->size--;
  queue->read_pos = (queue->read_pos + 1) % MAX_QUEUE_SIZE;  // queue->max_size;

  return 1;
}

/** @brief Функция для определения факта переполнения очережи
 * @param queue - указатель на очередь
 * @return Возвращает 1 в случае переполнения.
 */
int queue_overflow(queue_t *queue) {
  assert(queue);
  return queue->size >= MAX_QUEUE_SIZE;
}

/** @brief Функция для  определения размера очереди
 * @param queue - указатель на очередьмент
 * @return Возвращает размер очереди
 */
int queue_get_size(queue_t *queue) {
  assert(queue);
  return queue->size;
}

/** @brief Функция для сброса очереди
 * @param queue - указатель на очередь
 */
void queue_flush(queue_t *queue) {
  assert(queue);
  queue->read_pos = queue->write_pos = queue->size = 0;
}

/** @brief Функция для уничтоженя очереди
 * @param queue - указатель на очередь
 */
void queue_free(queue_t *queue) {
  assert(queue);
  free(queue);
}

// Вывод сообщений, связанных с поключение из очереди
void queue_invalidate_recs(queue_t *queue, connection_t const *conn) {
  int i;

  assert(queue);
  assert(conn);

  // Цикл по всем сообщениям в очереди.
  // Если указатель подключения в сообщении совпадает с указателм
  // conn, сообщению будет установлена некорректная метка времени
  // а так же обнулен указатель на подключение - источник,
  // что исключит его из очереди обработки.
  for (i = queue->read_pos; i != queue->write_pos;
       i = (i + 1) % MAX_QUEUE_SIZE) {
    // "Повредить" метку времени и указатель на коннект
    if (queue->data[i].src_connection == conn) {
      queue->data[i].timestamp = (struct timeval){0, 0};
      queue->data[i].src_connection = 0;
    }
  }
}
