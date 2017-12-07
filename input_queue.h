#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#ifndef GLOBAL_H_INCLUDED
#include "global.h"
#endif  // GLOBAL_H_INCLUDED

#ifndef MESSAGE_H_INCLUDED
#include "message.h"
#endif

typedef struct queue_t queue_t;

/**@struct queue_t
 * @brief Очередь FIFO
 */
struct queue_t {
  message_t data[MAX_QUEUE_SIZE]; /**< Буфер данных*/

  uint16_t read_pos;  /**< Позиция для чтения */
  uint16_t write_pos; /**< Позиция для записи */

  uint16_t size; /**< Текущий размер очереди */
};

// queue_t * queue_create();
int queue_init(queue_t *queue);
int queue_push(queue_t *queue, const void *src);
int queue_pop(queue_t *queue, void *dst);

int queue_overflow(queue_t *queue);
int queue_get_size(queue_t *queue);

void queue_invalidate_recs(queue_t *queue, connection_t const *conn);

void queue_flush(queue_t *queue);

void queue_free(queue_t *queue);

#endif  // QUEUE_H_INCLUDED
