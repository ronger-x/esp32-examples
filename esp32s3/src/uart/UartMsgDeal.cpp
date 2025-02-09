//
// Created by ronger on 2025/2/7.
//
#include "Arduino.h"
#include "UartMsgDeal.h"

// 构造函数
UartMsgDeal::UartMsgDeal(HardwareSerial& serial)
    : serial_(serial) {
}

void UartMsgDeal::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms, uint8_t rxfifo_full_thrhd)
{
    serial_.setRxBufferSize(RD_BUF_SIZE * 2);
    serial_.setTxBufferSize(BUF_SIZE * 2);
    serial_.begin(baud, config, rxPin, txPin, invert, timeout_ms, rxfifo_full_thrhd);
#ifdef CONFIG_AUTOSTART_ARDUINO
    // 注册数据接收回调（基于 C++ 的 lambda 表达式）
    serial_.onReceive([this]() { this->receiveDataTask(); });
#endif

}

void UartMsgDeal::receiveDataTask()
{
    uint8_t* buf = (uint8_t*)malloc(RD_BUF_SIZE);
    if (!buf)
    {
        return;
    }
    size_t toRead = 0, toWrite = 0, written = 0;
    size_t available = serial_.available();
    while (available)
    {
        toRead = (available > RD_BUF_SIZE) ? RD_BUF_SIZE : available;
        toWrite = serial_.readBytes(buf, toRead);
        written += serial_.write(buf, toWrite);
        available = serial_.available();
    }
    free(buf);
}