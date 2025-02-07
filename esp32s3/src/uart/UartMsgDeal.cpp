//
// Created by ronger on 2025/2/7.
//
#include "Arduino.h"
#include "UartMsgDeal.h"

void UartMsgDeal::begin() {
    Serial.onReceive(this->receiveDataTask(), true);
}

std::function<void()> UartMsgDeal::receiveDataTask() {
    uint8_t *buf = (uint8_t *) malloc(RD_BUF_SIZE);
    if (!buf) {
        return nullptr;
    }
    size_t toRead = 0, toWrite = 0, written = 0;
    size_t available = Serial.available();
    while (available) {
        toRead = (available > RD_BUF_SIZE) ? RD_BUF_SIZE : available;
        toWrite = Serial.readBytes(buf, toRead);
        written += Serial.write(buf, toWrite);
        available = Serial.available();
    }
    free(buf);
    return nullptr;
}