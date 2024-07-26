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

    printf("string to solve(formated):\n");
    for(int i = 0; i < 32; i++)
    {
        msg_to_solve[i] = dr[5 + i];
        if(i == 0)
        {
            printf("[0x%02x, ", msg_to_solve[i]);
        }
        else if(i < 31 && i > 0)
        {
            printf("0x%02x, ", msg_to_solve[i]);
        }
        else
        {
            printf("0x%02x];\n", msg_to_solve[i]);
        }
    }


    printf("string to solve:\n");
    for(int i = 0; i < 32; i++)
    {
        if(i < 31)
        {
            printf("%02x", msg_to_solve[i]);
        }
        else
        {
            printf("%02x\n", msg_to_solve[i]);
        }
    }

    uint8_t key[1] = {0x00};
    uint8_t *hmac_str = hmac(key, 1, msg_to_solve, 32);
    
    printf("hmac from FIPS eletra:\n");
    for(int i = 0; i < 32; i++)
    {
       if(i < 31)
       {
           printf("%02x | ", hmac_str[i]);
       }
       else
       {
           printf("%02x\n", hmac_str[i]);
       }
    }

    uint8_t t[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t type = MBEDTLS_MD_SHA256;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char *)key, 1);
    mbedtls_md_hmac_update(&ctx, msg_to_solve, 32);
    mbedtls_md_finish(&ctx, t);
    mbedtls_md_free(&ctx);

    printf("hmac from mbedtls:\n");
    for(int i = 0; i < 32; i++)
    {
       if(i < 31)
       {
           printf("%02x | ", t[i]);
       }
       else
       {
           printf("%02x\n", t[i]);
       }
    }

    set(ds, 0x11, hmac_str, 32);

    printf("data to send: \n");
    for(int i = 0; i < 66; i++)
    {
        if(i < 64)
        {
            printf("%02x | ", ds[i]);
        }
        else
        {
            printf("%02x\n", ds[i]);
        }
    }
    
    int code;
    for(int a = 0; a < 2; a++)
    {
        code = send_solved_string(ds, dr, 66, 258, 32);
        if(code == 2)
        {
            break;
        }
    }

    // uint8_t key[1] = {0x00};

    // uint8_t msg[] = {0x4E, 0x49, 0x9F, 0x08, 0x83, 0x2A, 0x47, 0x90, 0x30, 0x66, 
    //     0x80, 0xE6, 0xFE, 0xA4, 0xFB, 0x50, 0xD6, 0xFB, 0x08, 0xE2, 0xA0, 0xAE, 0x53, 0x3F, 0xFC, 0xED, 0xA5, 0x55, 0x0D, 0x97, 0xA8, 0x23};

    // uint8_t key[1];
    // key[0] = 0x00;
    // uint8_t *htest = hmac(key, 1, msg, 32);
    

    // for(int i = 0; i < 32; i++)
    // {
    //     printf("%02x-", htest[i]);
    // }
    // printf("\n");

    
    // send(ads, dr, 66, 258, 20);
    // int aux;
    // for(int i = 0; i < 10; i++)
    // {
    //     aux = send_solved_string(ds, adr, 66, 258, 10);
    //     if(aux == 0)
    //     {
    //         break;
    //     }
    // }
    // if(aux == -1)
    // {
    //     printf("A\n");
    // }


    //After trying to open session: 11 if you got it, 20 if you didn't
}