#include <Arduino.h>
#include <freertos/task.h>
#include "UserConfig.h"
#include "uart/UartMsgDeal.h"

void welcome();

void hardware_default_init();

void platform_init();

void task_init(void *p_arg);

void setup() {
    // write your initialization code here
    hardware_default_init();
    /* 版本信息 */
    welcome();
    /* 创建启动任务 */
    xTaskCreate(task_init, "init task", 280, nullptr, 4, nullptr);

    /* 启动调度，开始执行任务 */
     vTaskStartScheduler();
}

void loop() {
    // write your code here
}


void hardware_default_init() {
    // 初始化串口
    Serial.setRxBufferSize(RD_BUF_SIZE * 2);
    Serial.setTxBufferSize(BUF_SIZE * 2);
    Serial.begin(UART_PROTOCOL_BAUD_RATE);
}

void welcome() {
    Serial.print("\r\n");
    Serial.print("\r\n");
    Serial.print("\033[1;32m");
    Serial.println("RYMCU Esp32 SDK");


    // 输出 CPU 信息
    Serial.println("================ CPU =================");
    //    Serial.print("Chip Revision: ");
    //    Serial.println(ESP.getChipRevision());
    Serial.print("Chip Model: ");
    Serial.println(ESP.getChipModel());
    //    Serial.print("Chip Cores: ");
    //    Serial.println(ESP.getChipCores());
    //    Serial.print("CPU Frequency: ");
    //    Serial.print(ESP.getCpuFreqMHz());
    //    Serial.println(" MHz");

    // 输出 Internal RAM 使用情况
    Serial.println("================ Internal RAM =================");
    Serial.print("Total Heap Size: ");
    Serial.print(ESP.getHeapSize() / 1024);
    Serial.println(" KB");
    Serial.print("Available Heap: ");
    Serial.print(ESP.getFreeHeap() / 1024);
    Serial.println(" KB");
    //lowest level of free heap since boot
    Serial.print("Min Free Heap: ");
    Serial.print(ESP.getMinFreeHeap() / 1024);
    Serial.println(" KB");
    //largest block of heap that can be allocated at once
    Serial.print("Max Alloc Heap: ");
    Serial.print(ESP.getMaxAllocHeap() / 1024);
    Serial.println(" KB");

    // 输出 SPI RAM 使用情况
    Serial.println("================ SPI RAM =================");
    Serial.print("Total PSRAM: ");
    Serial.print(ESP.getPsramSize() / 1024);
    Serial.println(" KB");
    Serial.print("Free PSRAM: ");
    Serial.print(ESP.getFreePsram() / 1024);
    Serial.println(" KB");
    Serial.print("Min Free PSRAM: ");
    Serial.print(ESP.getMinFreePsram() / 1024);
    Serial.println(" KB");
    Serial.print("Max Alloc PSRAM: ");
    Serial.print(ESP.getMaxAllocPsram() / 1024);
    Serial.println(" KB");

    // 输出 Flash 使用情况
    Serial.println("================ Flash =================");
    Serial.print("Flash Size: ");
    Serial.print(ESP.getFlashChipSize() / 1024);
    Serial.println(" KB");
    //    Serial.print("Flash Chip Speed: ");
    //    Serial.print(ESP.getFlashChipSpeed() / 1000 / 1000);
    //    Serial.println(" MHz");
    //    Serial.print("Flash Chip Mode: ");
    //    Serial.println(ESP.getFlashChipMode());
    Serial.print("\r\n");
    Serial.print("\r\n");
}

void platform_init() {
}

void task_init(void *p_arg) {
#if UART_PROTOCOL_DEBUG
    // 初始化 UART 调试串口信息处理程序
//    UartMsgDeal uartMsgDeal;
//    uartMsgDeal.begin();
#endif
}
