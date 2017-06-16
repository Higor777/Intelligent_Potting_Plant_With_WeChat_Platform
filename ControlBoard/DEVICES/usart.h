#ifndef __USART_H__
#define __USART_H__
#include "sys.h"
#include "queue.h"

#define USART1_TX_EN    1
#define USART1_RX_EN    1
#define USART2_TX_EN    0
#define USART2_RX_EN    0
#define USART3_TX_EN    0
#define USART3_RX_EN    0
#define UART4_TX_EN     0
#define UART4_RX_EN     0
#define UART5_TX_EN     0
#define UART5_RX_EN     0

#define USART1_BaudRate 115200
#define USART2_BaudRate 115200
#define USART3_BaudRate 115200
#define UART4_BaudRate  115200
#define UART5_BaudRate  115200

#define USART1TxMaxBuffLen  1024
#define USART1RxMaxBuffLen  1024
#define USART2TxMaxBuffLen  1024
#define USART2RxMaxBuffLen  1024
#define USART3TxMaxBuffLen  1024
#define USART3RxMaxBuffLen  1024
#define UART4TxMaxBuffLen   1024
#define UART4RxMaxBuffLen   1024
#define UART5TxMaxBuffLen   1024
#define UART5RxMaxBuffLen   1024

//for printf(const char *format , ...)
#define printf_usart USART1

typedef struct dma_usart_ch
{
	volatile Queue rx_queue;			//接收数据缓冲队列，DMA将串口接收到的数据存入此队列
	volatile Queue tx_queue;			//发送数据缓冲队列，DMA将此队列的数据传输到串口
	uint8_t Flag_Tx_Busy;				//传输忙标志
	uint8_t Flag_Rx_IDLE;				//接收空闲标志
	uint8_t tx_queue_full;				//发送队列标志
    uint8_t dmaSendErrorCount;          //DMA发送意外停止次数，仅用于调试
    uint8_t dmaRecErrorCount;          //DMA发送意外停止次数，仅用于调试
}DMA_USART_Ch;



typedef enum task_prio_t {
	TASK_PRIO_Start_task = 3,
	TASK_PRIO_Idle_task = 10, //暂未使用
	
	TASK_PRIO_WatchDog_task = 3,
	TASK_PRIO_DistributeMsg_Hander_task = 4,
	TASK_PRIO_MAVLinkMsg_Hander_task = 4,
	TASK_PRIO_RTCMMsg_Hander_task = 4,
	TASK_PRIO_nmea_GPSMsg_Hander_task = 4,
	TASK_PRIO_DEBUG_MAVLinkMsg_Hander_task = 4,
    TASK_PRIO_CAN1Msg_Hander_task = 4

} task_prio_t;

typedef enum interrupt_preempt_prio_t {
	INT_PREEMPT_PRIO_CAN1_RX0 = 3,
	INT_PREEMPT_PRIO_USART1 = 3, //未使能
	INT_PREEMPT_PRIO_USART2 = 3, //未使能
	INT_PREEMPT_PRIO_USART3 = 3, //未使能
	INT_PREEMPT_PRIO_UART4  = 3, //未使能
	INT_PREEMPT_PRIO_UART5  = 2, //使能接收
	INT_PREEMPT_PRIO_DMA1_Channal4  = 2, //U1_TX
	INT_PREEMPT_PRIO_DMA1_Channal5  = 2, //U1_RX
	INT_PREEMPT_PRIO_DMA1_Channal7  = 1, //U2_TX
	INT_PREEMPT_PRIO_DMA1_Channal6  = 1, //U2_RX
	INT_PREEMPT_PRIO_DMA1_Channal2  = 3, //U3_TX
	INT_PREEMPT_PRIO_DMA1_Channal3  = 2, //U3_RX
    INT_PREEMPT_PRIO_DMA2_Channal4_5 = 3,//U4_TX
	INT_PREEMPT_PRIO_DMA2_Channal3  = 3 //U4_RX
	
} interrupt_preempt_prio_t;

typedef enum interrupt_sub_prio_t {
	INT_SUB_PRIO_CAN1_RX0 = 3,
	INT_SUB_PRIO_USART1 = 3, //未使能
	INT_SUB_PRIO_USART2 = 3, //未使能
	INT_SUB_PRIO_USART3 = 3, //未使能
	INT_SUB_PRIO_UART4  = 3, //未使能
	INT_SUB_PRIO_UART5  = 2, //使能接收
	INT_SUB_PRIO_DMA1_Channal4  = 3, //U1_TX
	INT_SUB_PRIO_DMA1_Channal5  = 0, //U1_RX
	INT_SUB_PRIO_DMA1_Channal7  = 0, //U2_TX
	INT_SUB_PRIO_DMA1_Channal6  = 0, //U2_RX
	INT_SUB_PRIO_DMA1_Channal2  = 0, //U3_TX
	INT_SUB_PRIO_DMA1_Channal3  = 1, //U3_RX
    INT_SUB_PRIO_DMA2_Channal4_5 = 3,//U4_TX
	INT_SUB_PRIO_DMA2_Channal3  = 1 //U4_RX
	
} interrupt_sub_prio_t;

void USART_printf(USART_TypeDef* USARTx, const char *format , ...);
void USART_Module_Init(void);

void USART_Send(USART_TypeDef* USARTx,uint8_t * data, uint16_t length);
static void USART_Queue_Init(void);
void usart_dma_send_init(void);
void dma_send(USART_TypeDef* USARTx,uint8_t *data,uint16_t size);
void usart_dma_receive_init(void);
void USART_Configuration(void);


#endif




