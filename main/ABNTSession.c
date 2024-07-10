#include "ABNTSession.h"
#include <esp_log.h>

const char *UART_TAG = "UART";

void setup_uart()
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

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, TX_BUF * 2, RX_BUF * 2, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    uart_set_line_inverse(UART_NUM_2, UART_SIGNAL_TXD_INV | UART_SIGNAL_RXD_INV);
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 18, 5, 4, 2));
}

uint16_t replaceByte(uint16_t value, uint8_t b, uint16_t pos)
{
    return (value & ~(0xFF << (pos * 8))) | ((b & 0xFF) << (pos * 8));
}

uint16_t crc16arc_bit(uint16_t crc, void const *mem, size_t len) {
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

int uart_send(void *data)
{
    int uart_retries = 0;
    UARTData *d = (UARTData*)data;
    
    uint16_t crc_dr = 0;
    uint16_t crc_checker = 0;
    printf("What\n");

    for(int i = 0; i < 66; i++)
    {
        if(i < 65)
        {
            printf("%x", d->ds[i]);
        }
        else
        {
            printf("%x\n", d->ds[i]);
        }
    }

    while(uart_retries <= 20)
    {
        uart_write_bytes(UART_NUM_2, (void *)d->ds, d->ds_size);
        int code = uart_read_bytes(UART_NUM_2, d->dr, (d->dr_size - 1), 100 / portTICK_PERIOD_MS);

        if(code > -1)
        {
            crc_dr = replaceByte(crc_dr, d->dr[d->dr_size - 2], 1);
            crc_dr = replaceByte(crc_dr, d->dr[d->dr_size - 1], 0);
            crc_checker = crc16arc_bit(0, d->dr, 256);
            printf("uart retries: %i\n", uart_retries);

            if(crc_dr == crc_checker && d->ds[0] != 0)
            {
                ESP_LOGI(UART_TAG, "Message validated!");
                break;
            }
            else
            {
                uart_retries++;
                ESP_LOGE(UART_TAG, "Message invalid");
            }
        }
        else
        {
            uart_retries++;
            ESP_LOGE(UART_TAG,"Couldn't get UART message. num of retries: %i", uart_retries);
        }
        uart_flush(UART_NUM_2);
        vTaskDelay(500);
    }

    if(uart_retries >= 20)
    {
        return 1;
    }

    return 0;
}

uint8_t *get_abnt_byte_array(CommandQueue *cq, int command)
{
    return (uint8_t *)command_get_data(cq, command)->data;
}

void open_abnt_session(void *data)
{
    UARTData *d = (UARTData *)data;
    uint8_t weekday_payload[4] = {0x13, 0x02, 0x24, 0x01}; 

    set_abnt_data(d, 0x29, weekday_payload, 4);

    for(int i = 0; i < 66; i++)
    {
        if(i < 65)
        {
            printf("%x | ", d->ds[i]);
        }
        else
        {
            printf("%x\n", d->ds[i]);
        }
    }
    uart_send((void *)d);
    //printf("string to solve: %s\n", d->dr);
    // set_abnt_data(d, 0x37, NULL, 0);
    // uart_send((void *)d);
    // set_abnt_data(d, 0x13, NULL, 0);
    // uart_send((void *)d);
    // printf("string to solve: %s\n", d->dr);
}

UARTData *uart_data_as_abnt()
{
    UARTData *data = (UARTData *)malloc(sizeof(UARTData));

    data->ds = (uint8_t *)calloc(68, sizeof(uint8_t));
    data->ds_size = 66;
    data->dr = (uint8_t *)calloc(258, sizeof(uint8_t));
    data->dr_size = 0;

    return data;
}

void set_abnt_data(UARTData *data, uint8_t command, uint8_t *payload, uint8_t payload_len)
{
    for(int i = 0; i < 64; i++)
    {
        switch(i)
        {
            case 0:
                data->ds[i] = command;
            break;

            case 1:
                data->ds[i] = 0x12;
            break;

            case 2:
                data->ds[i] = 0x34;
            break;
                    
            case 3:
                data->ds[i] = 0x56;
            break;
        
            default:
                data->ds[i] = 0x00;
            break;
        }
    }

    if(payload != NULL)
    {
        for(int i = 0; i < payload_len; i++)
        {
            data->ds[4 + i] = payload[i];
        }
    }

    uint16_t a = crc16arc_bit(0, data, 64);
    data->ds[64] = (a >> (1 * 8)) & 0xFF;
    data->ds[65] = (a >> (0 * 8)) & 0xFF;

    for(int i = 0; i < 66; i++)
    {
        if(i < 65)
        {
            printf("%x | ", data->ds[i]);
        }
        else
        {
            printf("%x\n", data->ds[i]);
        }
    }
}