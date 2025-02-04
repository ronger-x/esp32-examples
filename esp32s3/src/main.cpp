#include <Arduino.h>
#include "UserConfig.h"
#include "UartMsgDeal.h"

#include <uart.h>         //先把uart.h复制工程include目录下，//#include "C:\Users\Hugh\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.11\tools\sdk\esp32s3\include\driver\include\driver\uart.h"
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
#define EX_UART_NUM UART_NUM_1 // set uart1
#define PATTERN_CHR_NUM (3)

#define UART_PIN_TX GPIO_NUM_11  // 串口发送引脚GPIO_11
#define UART_PIN_RX GPIO_NUM_12 // 串口接收引脚GPIO_12

static QueueHandle_t uart1_queue; // 串口接收队列,当串口完成数据接收后，发送消息给该队列，只需在线程等待该消息队列完成数据处理
void uart_init(void);                             // 串口初始化
static void uart_event_task(void *pvParameters); // 接收串口处理函数

void welcome();
void platform_init();
static void task_init(void* p_arg);

void setup()
{
    // write your initialization code here
    Serial.begin(UART_PROTOCOL_BAUD_RATE);
    /* 版本信息 */
    //welcome();
    uart_init();          // 初始化接收串口
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL); // 处理串口消息
    /* 创建启动任务 */
    //xTaskCreate(task_init, "init task", 280, nullptr, 4, nullptr);

    /* 启动调度，开始执行任务 */
    //vTaskStartScheduler();
}

void loop()
{
     Serial.println("hello loop.");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
}

void welcome()
{
    Serial.print("\r\n");
    Serial.print("\r\n");
    Serial.print("\033[1;32m");
    Serial.println("RYMCU Esp32 SDK");

/*
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
    */
}

void platform_init()
{
}

static void task_init(void* p_arg)
{
#if UART_PROTOCOL_DEBUG
    // 初始化 UART 调试串口信息处理程序
    UartMsgDeal uartMsgDeal;
    uartMsgDeal.uartMsgTaskInitial();
#endif
}

/*************************  串口初始化  ****************************/
void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart1_queue, 0); // 安装串口驱动，并关联队列uart1_queue
    uart_param_config(EX_UART_NUM, &uart_config);

    uart_set_pin(EX_UART_NUM, UART_PIN_TX, UART_PIN_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); // 设置串口引脚（TX:18,RX:19）
    uart_enable_pattern_det_baud_intr(EX_UART_NUM, '+', PATTERN_CHR_NUM, 9, 0, 0);               // Set uart pattern detect function.
    uart_pattern_queue_reset(EX_UART_NUM, 20);                                                   // Reset the pattern queue length to record at most 20 pattern positions.
}
/*************************  串口中断事件处理线程  ****************************/
static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t *dtmp = (uint8_t *)malloc(RD_BUF_SIZE);
    while (1)
    {
        if (xQueueReceive(uart1_queue, (void *)&event, (TickType_t)portMAX_DELAY))
        {
            bzero(dtmp, RD_BUF_SIZE);
            switch (event.type)
            {

            case UART_DATA:
                uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY); // 读出接收到的数据
                uart_write_bytes(EX_UART_NUM, (const char *)dtmp, event.size); // 打印接收到的数据
                Serial.printf("[UART DATA]: %d", event.size);
                Serial.printf("[UART DATA]: %d,%d,%d", dtmp[0], dtmp[1], dtmp[2]);
                // if (event.size == 4)
                //  test(dtmp);
                // Uart_data_process(dtmp, event.size); // 处理接收到的数据
                //Get_CMD_NOW(dtmp, event.size);
                break;
            default:
                Serial.printf("uart event type: %d", event.type);
                break;
            }
        }
        taskYIELD();
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}


