#include <Arduino.h>
#include <freertos/task.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <wifi/WIFIHandle.h>

#include "UserConfig.h"
#include "uart/UartMsgDeal.h"

UartMsgDeal uartMsgDeal(Serial);

WIFIHandle wifiHandle;

void welcome();

void hardware_default_init();

void platform_init();

void task_init(void* p_arg);

void setup()
{
    // write your initialization code here
    hardware_default_init();
    /* 版本信息 */
    welcome();
    /* 创建启动任务 */
    xTaskCreateUniversal(task_init, "task_init", ARDUINO_SERIAL_EVENT_TASK_STACK_SIZE * 2, nullptr,
                         ARDUINO_SERIAL_EVENT_TASK_PRIORITY, nullptr, ARDUINO_SERIAL_EVENT_TASK_RUNNING_CORE);
}

void loop()
{
    // write your code here
    vTaskDelay(10000 / portTICK_PERIOD_MS);
}


void hardware_default_init()
{
    // 初始化 NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    // 初始化串口
    uartMsgDeal.begin(UART_PROTOCOL_BAUD_RATE);
}

void welcome()
{
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

void platform_init()
{
}

void task_init(void* p_arg)
{
    // 初始化 Wi-Fi
#ifdef WIFI_ENABLED
    wifiHandle.begin();
#endif
    vTaskDelete(nullptr);
}
