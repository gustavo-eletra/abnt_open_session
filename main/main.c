#include <stdio.h>
#include <string.h>
#include <mbedtls/md.h>
#include <mbedtls/sha256.h>
#include "uart.h"
#include "hmac.h"

uint8_t msg_to_solve[32];
uint8_t ds[66];
uint8_t ads[66];
uint8_t *dr;
uint8_t *adr;

QueueHandle_t u_queue;

void app_main(void)
{
    setup_uart(&u_queue);
    dr = (uint8_t *)calloc(RX_BUF, sizeof(uint8_t));
    set(ds, 0x14, NULL, 0);
    send(ds, dr, 66, 258, 10);

    uint8_t weekday_payload[4] = {0x13, 0x02, 0x24, 0x01}; 
    set(ds, 0x29, weekday_payload, 4);
    send(ds, dr, 66, 258, 3);
    set(ds, 0x37, NULL, 0);
    send(ds, dr, 66, 258, 10);
    set(ds, 0x13, NULL, 0);
    send(ds, dr, 66, 258, 10);

    for(int i = 0; i < 32; i++)
    {
        msg_to_solve[i] = dr[5 + i];
    }

    uint8_t key[1] = {0x00};
    uint8_t *hmac_str = hmac(key, 1, msg_to_solve, 32);

    set(ds, 0x11, hmac_str, 32);
   
    int code;
    for(int a = 0; a < 2; a++)
    {
        code = send_solved_string(ds, dr, 66, 258, 32);
        if(code == 2)
        {
            break;
        }
    }


    set(ds, 0x30, NULL, 0);
    send(ds, dr, 66, 258, 10);
   
}