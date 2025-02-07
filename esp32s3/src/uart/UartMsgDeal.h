//
// Created by ronger on 2025/2/7.
//

#ifndef UART_MSG_DEAL_H
#define UART_MSG_DEAL_H

#include <functional>

#define BUF_SIZE 1024
#define RD_BUF_SIZE (BUF_SIZE)

class UartMsgDeal {
    public:
        void begin();
        std::function<void()> receiveDataTask();
};

#endif //UART_MSG_DEAL_H
