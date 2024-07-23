#include "uart.h"

const char *UART_TAG = "UART";

void setup_uart(QueueHandle_t *uart_queue)
{
    uart_config_t uart_config = 
    {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, RX_BUF * 2, TX_BUF * 2, 50, uart_queue, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    uart_set_line_inverse(UART_NUM_2, UART_SIGNAL_TXD_INV | UART_SIGNAL_RXD_INV);
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 18, 5, 4, 2));
}

uint16_t replaceByte(uint16_t value, uint8_t b, uint16_t pos)
{
    return (value & ~(0xFF << (pos * 8))) | ((b & 0xFF) << (pos * 8));
}

uint16_t crc16arc_bit(uint16_t crc, void const *mem, size_t len)
{
    unsigned char const *data = mem;
    if (data == NULL)
        return 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (unsigned k = 0; k < 8; k++) {
            crc = crc & 1 ? (crc >> 1) ^ 0xa001 : crc >> 1;
        }
    }

    uint32_t tmp = crc;
    crc = (crc << 8) | (tmp >> 8 & 0xff);
    return crc;
}

int send(uint8_t *s, uint8_t *r, uint16_t size_of_s, uint16_t size_of_r, uint8_t retries)
{
    int uart_retries = 0;
    
    uint16_t crc_dr = 0;
    uint16_t crc_checker = 0;

    while(uart_retries <= retries)
    {
        crc_dr = 0;
        crc_checker = 0;
        uart_write_bytes(UART_NUM_2, s, size_of_s);
        ESP_ERROR_CHECK(uart_wait_tx_done(UART_NUM_2, 100));
        int code = uart_read_bytes(UART_NUM_2, r, (RX_BUF - 1), 100 / portTICK_PERIOD_MS);

        if(code > -1)
        {
            for(int i = 0; i < 258; i++)
            {
                if(i < 257)
                {
                    printf("%x | ", r[i]);
                }
                else
                {
                    printf("%x\n", r[i]);
                }
            }

            crc_dr = replaceByte(crc_dr, r[size_of_r - 2], 1);
            crc_dr = replaceByte(crc_dr, r[size_of_r - 1], 0);
            crc_checker = crc16arc_bit(0, r, 256);
            printf("msg crc: %x%x\n", r[size_of_r - 2], r[size_of_r - 1]);
            printf("crc checker: %x\n", crc_checker);
            printf("uart retries: %i\n", uart_retries);

            if(crc_dr == crc_checker)
            {
                ESP_LOGI("UART", "Message validated!");
                break;
            }
            else
            {
                uart_retries++;
                ESP_LOGE("UART", "Message invalid");
            }
        }
        else
        {
            uart_retries++;
            ESP_LOGE("UART","Couldn't get UART message. num of retries: %i", uart_retries);
        }
        vTaskDelay(50);
    }

    uart_flush(UART_NUM_2);
    if(uart_retries >= retries)
    {
        return -1;
    }
    return r[0];
}

void set(uint8_t *data, uint8_t command, uint8_t *payload, uint8_t payload_len)
{
    for(int i = 0; i < 66; i++)
    {
        switch(i)
        {
            case 0:
                data[i] = command;
            break;

            case 1:
                data[i] = 0x12;
            break;

            case 2:
                data[i] = 0x34;
            break;
                    
            case 3:
                data[i] = 0x56;
            break;
        
            default:
                data[i] = 0x00;
            break;
        }
    }

    if(payload != NULL)
    {
        for(int i = 0; i < payload_len; i++)
        {
            data[4 + i] = payload[i];
        }
    }


    for(int i = 4 + payload_len; i < 64; i++)
    {
        data[i] = 0x00;
    }
    
    uint16_t a = crc16arc_bit(0, data, 64);
    data[64] = (a >> (1 * 8)) & 0xFF;
    data[65] = (a >> (0 * 8)) & 0xFF;

    // for(int i = 0; i < 66; i++)
    // {
    //     if(i < 65)
    //     {
    //         printf("%x | ", data[i]);
    //     }
    //     else
    //     {
    //         printf("%x\n", data[i]);
    //     }
    // }
}


int send_solved_string(uint8_t *s, uint8_t *r, uint16_t size_of_s, uint16_t size_of_r, uint8_t retries)
{
    int uart_retries = 0;
    
    uint16_t crc_dr = 0;
    uint16_t crc_checker = 0;

     uart_write_bytes(UART_NUM_2, s, size_of_s);
    ESP_ERROR_CHECK(uart_wait_tx_done(UART_NUM_2, 100));   
    while(uart_retries <= retries)
    {
        int code = uart_read_bytes(UART_NUM_2, r, (RX_BUF - 1), 100 / portTICK_PERIOD_MS);

        if(code > -1)
        {
            for(int i = 0; i < 258; i++)
            {
                if(i < 257)
                {
                    printf("%x | ", r[i]);
                }
                else
                {
                    printf("%x\n", r[i]);
                }
            }

            crc_dr = replaceByte(crc_dr, r[size_of_r - 2], 1);
            crc_dr = replaceByte(crc_dr, r[size_of_r - 1], 0);
            crc_checker = crc16arc_bit(0, r, 256);
            printf("command sent: %x\n", s[0]);
            printf("msg command: %x\n", r[0]);
            printf("msg crc: %x%x\n", r[size_of_r - 2], r[size_of_r - 1]);
            printf("crc checker: %x\n", crc_checker);
            printf("uart retries: %i\n", uart_retries);

            if(crc_dr == crc_checker && r[0] == 0x11)
            {
                ESP_LOGI("UART", "Message validated!");
                break;
            }
            else
            {
                uart_retries++;
                ESP_LOGE("UART", "Message invalid");
            }
        }
        else
        {
            uart_retries++;
            ESP_LOGE("UART","Couldn't get UART message. num of retries: %i", uart_retries);
        }

        vTaskDelay(50);
    }

    if(uart_retries >= retries)
    {
        return -1;
    }

    return 0;
}