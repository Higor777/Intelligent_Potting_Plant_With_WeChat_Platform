#ifndef   QUEUE_H_
#define   QUEUE_H_

#include "sys.h"


typedef struct{
    uint8_t * data;
    int  head,tail;
    uint32_t Length;
}Queue;

void queue_init(volatile Queue * pQueue);
uint8_t queue_get(volatile Queue * pQueue);
void queue_put(volatile Queue * pQueue, uint8_t ch);
void queue_putarr(volatile Queue * pQueue, uint8_t * ch ,uint16_t len);
int queue_empty(volatile Queue * queue);
int queue_full(volatile Queue * queue);
void queue_tail_inc(volatile Queue * pQueue);

#endif

