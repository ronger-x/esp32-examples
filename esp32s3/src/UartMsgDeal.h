//
// Created by ronger on 2025/2/4.
//

#ifndef UART_MSG_DEAL_H
#define UART_MSG_DEAL_H

#include <driver/uart.h>

#define UART_PROTOCOL_NUMBER          (UART_NUM_0) // 串口通讯协议编号
#define UART_PROTOCOL_BAUD_RATE       115200 // 串口通讯协议波特率
#ifndef UART_PROTOCOL_DEBUG
#define UART_PROTOCOL_DEBUG           0 // 串口通讯调试功能开关
#endif
#ifndef UART_PROTOCOL_VERSION
#define UART_PROTOCOL_VERSION         1 // 串口通讯协议版本
#endif

#define BUF_SIZE    1024           // DMA缓冲区大小
#define RD_TIMEOUT      10             // 接收超时（单位：RTOS ticks）
#define RD_BUF_SIZE (BUF_SIZE)
#define QUEUE_SIZE      20              // 队列中最大数据包数量
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

class UartMsgDeal {
    public:
        void uartMsgTaskInitial();
};

#endif //UART_MSG_DEAL_H
