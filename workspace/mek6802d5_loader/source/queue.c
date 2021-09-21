/*
 * queue.c
 *
 *  Created on: Sep 17, 2021
 *      Author: Jim
 */

#include "queue.h"

bool queue_init (QUEUE *q)
{
	q->head = 0;
	q->tail = 0;

	return true;
}

bool queue_empty (QUEUE *q)
{
	return ((q->head == q->tail) ? true : false);
}

bool queue_enqueue (QUEUE *q, QUEUE_TYPE data)
{
	q->head++;
	if (q->head == q->tail)	// queue is full
	{
		q->head--;
		return false;
	}
	else if (q->head == QUEUE_SIZE)
	{
		if (q->tail == 0) // queue is full
		{
			q->head--;
			return false;
		}
		else  // we need to start back at the beginning
		{
			q->head = 0;
		}
	}
	q->array[q->head] = data;

	return true;
}

bool queue_dequeue (QUEUE *q, QUEUE_TYPE *data)
{
	if (q->head == q->tail) // queue is empty
		return false;
	q->tail++;
	if (q->tail == QUEUE_SIZE)
		q->tail = 0;
	*data = q->array[q->tail];

	return true;
}
