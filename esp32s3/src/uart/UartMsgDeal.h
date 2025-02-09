//
// Created by ronger on 2025/2/7.
//

#ifndef UART_MSG_DEAL_H
#define UART_MSG_DEAL_H

#define BUF_SIZE 1024
#define RD_BUF_SIZE (BUF_SIZE)

class UartMsgDeal
{
public:
    // 构造函数
    explicit UartMsgDeal(HardwareSerial& serial);

    // 初始化函数，用于配置串口
    // When pins are changed, it will detach the previous ones
    // if pin is negative, it won't be set/changed and will be kept as is
    // timeout_ms is used in baudrate detection (ESP32, ESP32S2 only)
    // invert will invert RX/TX polarity
    // rxfifo_full_thrhd if the UART Flow Control Threshold in the UART FIFO (max 127)
    void begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1, int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL, uint8_t rxfifo_full_thrhd = 112);

    // 串口接收任务
    void receiveDataTask();
private:
    HardwareSerial& serial_;
};

#endif //UART_MSG_DEAL_H
