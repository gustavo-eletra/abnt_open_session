#include <stdio.h>
#include <esp_efuse.h>

#include "hmac.h"
#include "ABNTSession.h"

uint8_t ds[66];
uint8_t *dr;

typedef enum
{
    OPEN_SESSION,
    SEND_COMMAND,
}COMMANDS;

int send(uint8_t *s, uint8_t *r, uint16_t size_of_s, uint16_t size_of_r, uint8_t retries)
{
    int uart_retries = 0;
    
    uint16_t crc_dr = 0;
    uint16_t crc_checker = 0;

    while(uart_retries <= retries)
    {
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
        vTaskDelay(100);
    }

    uart_flush(UART_NUM_2);

    if(uart_retries >= retries)
    {
        return -1;
    }

    return dr[0];
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

    // printf("data to after payload:\n");
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

    uint16_t a = crc16arc_bit(0, ds, 64);
    data[64] = (a >> (1 * 8)) & 0xFF;
    data[65] = (a >> (0 * 8)) & 0xFF;
}

// uint8_t *hmac(uint8_t key, uint8_t *message)
// {

// }

void app_main(void)
{
    setup_uart();
    dr = (uint8_t *)calloc(RX_BUF, sizeof(uint8_t));

    //Dry run
    set(ds, 0x14, NULL, 0);
    send(ds, dr, 66, 258, 10);

    //Check if command returns 0x40 as the first string
    uint8_t weekday_payload[4] = {0x13, 0x02, 0x24, 0x01}; 
    set(ds, 0x29, weekday_payload, 4);
    send(ds, dr, 66, 258, 10);
    set(ds, 0x37, NULL, 0);
    send(ds, dr, 66, 258, 10);
    set(ds, 0x13, NULL, 0);
    send(ds, dr, 66, 258, 10);

    uint8_t msg_to_solve[32];
    uint8_t *hmac_str;

    for(int i = 0; i < 32; i++)
    {
        msg_to_solve[i] = dr[5 + i];
    }

    uint8_t key[1] = {0x00};


    hmac_str = hmac(key, 1, msg_to_solve, 32);
    
    printf("hmac generated:\n");
    for(int i = 0; i < 32; i++)
    {
        if(i < 31)
        {
            printf("%x", hmac_str[i]);
        }
        else
        {
            printf("%x\n", hmac_str[i]);
        }
    }

    uint8_t ads[66];
    set(ds, 0x11, hmac_str, 32);
    send(ds, dr, 66, 258, 10);

    //After trying to open session: 11 if you got it, 20 if you didn't

    // CommandQueue *cq = create_command_queue(10);
    // UARTData *t_data = uart_data_as_abnt();
    // register_func_to_command_queue(cq, &open_abnt_session, t_data, sizeof(t_data), OPEN_SESSION);
    // enqueue_command_buffer(cq, OPEN_SESSION);
    // process_command_queue(cq);
}