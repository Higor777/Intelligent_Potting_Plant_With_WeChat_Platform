#include "queue.h"
#include <string.h>

void queue_init(volatile Queue * pQueue)
{
    memset((uint8_t *)pQueue, 0, sizeof(Queue));
}

uint8_t queue_get(volatile Queue * pQueue)
{
    uint8_t data = pQueue->data[pQueue->head];
    pQueue->head = (pQueue->head + 1) % pQueue->Length;
    return data;
}

void queue_put(volatile Queue * pQueue, uint8_t ch)
{
	if(!queue_full(pQueue))
	{
		pQueue->data[pQueue->tail] = ch;
		pQueue->tail = (pQueue->tail + 1) % pQueue->Length;
	}
}

//向队列添加多个元素
void queue_putarr(volatile Queue * pQueue, uint8_t * ch ,uint16_t len)
{
    uint16_t i = 0;
	for(i=0;i<len;i++)
	{
		queue_put(pQueue,*(ch+i));
	}
}


int queue_empty(volatile Queue * queue)
{
    if(queue->head == queue->tail)
        return 1;
    return 0;
}

int queue_full(volatile Queue * queue)
{
    if((queue->tail + 1) % queue->Length == queue->head)
        return 1;
    return 0;
}


//尾部加1，赋值由DMA进行
void queue_tail_inc(volatile Queue * pQueue)
{
	pQueue->tail = (pQueue->tail + 1) % pQueue->Length;
}

