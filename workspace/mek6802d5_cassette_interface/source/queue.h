/*
 * queue.h
 *
 *  Created on: Sep 17, 2021
 *      Author: Jim
 */

#include <stdio.h>
#include <stdint.h>
#include "board.h"
//#include "fsl_debug_console.h"

#ifndef QUEUE_H_
#define QUEUE_H_

#define QUEUE_SIZE 4096
typedef uint8_t QUEUE_TYPE;

struct _queue {
	uint32_t head;
	uint32_t tail;
	QUEUE_TYPE array[QUEUE_SIZE];
};

typedef struct _queue QUEUE;

bool queue_init (QUEUE *q);
bool queue_empty (QUEUE *q);
bool queue_enqueue (QUEUE *q, QUEUE_TYPE data);
bool queue_dequeue (QUEUE *q, QUEUE_TYPE *data);

#endif /* QUEUE_H_ */
