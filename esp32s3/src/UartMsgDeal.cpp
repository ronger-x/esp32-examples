//
// Created by ronger on 2025/2/4.
//
#include "UartMsgDeal.h"

#include <Arduino.h>
#include <esp_log.h>

static const char* TAG = "uart_events";

static QueueHandle_t uartMsgQueue;

/*************************  串口中断事件处理线程  ****************************/
static void uart_event_task(void* pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*)malloc(RD_BUF_SIZE);
    while (true)
    {
        //Waiting for UART event.
        if (xQueueReceive(uartMsgQueue, (void*)&event, (TickType_t)portMAX_DELAY))
        {
            bzero(dtmp, RD_BUF_SIZE);
            ESP_LOGI(TAG, "uart[%d] event:", UART_PROTOCOL_NUMBER);
            switch (event.type)
            {
            //Event of UART receiving data
            /*We'd better handler data event fast, there would be much more data events than
            other types of events. If we take too much time on data event, the queue might
            be full.*/
            case UART_DATA:
                {
                    ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                    uart_read_bytes(UART_PROTOCOL_NUMBER, dtmp, event.size, portMAX_DELAY);
                    ESP_LOGI(TAG, "[DATA EVT]:");
                    uart_write_bytes(UART_PROTOCOL_NUMBER, (const char*)dtmp, event.size);
                    break;
                }
            //Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                {
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(UART_PROTOCOL_NUMBER);
                    xQueueReset(uartMsgQueue);
                    break;
                }
            //Event of UART ring buffer full
            case UART_BUFFER_FULL:
                {
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider increasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(UART_PROTOCOL_NUMBER);
                    xQueueReset(uartMsgQueue);
                    break;
                }
            //Event of UART RX break detected
            case UART_BREAK:
                {
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                }
            //Event of UART parity check error
            case UART_PARITY_ERR:
                {
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                }
            //Event of UART frame error
            case UART_FRAME_ERR:
                {
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                }
            //UART_PATTERN_DET
            case UART_PATTERN_DET:
                {
                    uart_get_buffered_data_len(UART_PROTOCOL_NUMBER, &buffered_size);
                    int pos = uart_pattern_pop_pos(UART_PROTOCOL_NUMBER);
                    ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                    if (pos == -1)
                    {
                        // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                        // record the position. We should set a larger queue size.
                        // As an example, we directly flush the rx buffer here.
                        uart_flush_input(UART_PROTOCOL_NUMBER);
                    }
                    else
                    {
                        uart_read_bytes(UART_PROTOCOL_NUMBER, dtmp, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(UART_PROTOCOL_NUMBER, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                        ESP_LOGI(TAG, "read data: %s", dtmp);
                        ESP_LOGI(TAG, "read pat : %s", pat);
                    }
                    break;
                }
            //Others
            default:
                {
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
                }
            }
        }
    }
    free(dtmp);
    dtmp = nullptr;
    vTaskDelete(nullptr);
}

/**
 * @brief 串口消息任务资源初始化
 *
 */
void UartMsgDeal::uartMsgTaskInitial()
{
    uart_config_t uart_config = {
        .baud_rate = UART_PROTOCOL_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB
    };
    // 安装驱动程序
    uart_driver_install(
        UART_PROTOCOL_NUMBER, // UART编号
        BUF_SIZE * 2, // RX缓冲区大小
        BUF_SIZE * 2, // TX缓冲区大小
        QUEUE_SIZE, // 事件队列大小
        &uartMsgQueue, // 事件队列句柄
        0 // 中断分配标志
    );
    uart_param_config(UART_PROTOCOL_NUMBER, &uart_config);
    // 设置通信管脚
    uart_set_pin(UART_PROTOCOL_NUMBER, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    //Set uart pattern detect function.
    uart_enable_pattern_det_baud_intr(UART_PROTOCOL_NUMBER, '+', PATTERN_CHR_NUM, 9, 0, 0);
    //Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(UART_PROTOCOL_NUMBER, QUEUE_SIZE);
    //Create a task to handler UART event from ISR
    xTaskCreate(uart_event_task, "uart_event_task", 3072, nullptr, 12, nullptr);
}
