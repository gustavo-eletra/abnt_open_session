#include <string.h>
#include <mbedtls/sha256.h>
#include <freertos/FreeRTOS.h>
#include "hmac.h"

void update_pads(uint8_t *ipad, uint8_t *opad, uint8_t *key, int key_len)
{
    for(int i = 0; i < 64; i++)
    {
        ipad[i] = IPAD_VALUE;
        opad[i] = OPAD_VALUE;
    }

    for(int i = 0; i < key_len; ++i)
    {
        ipad[i] ^= key[i];
        opad[i] ^= key[i];
    }


    printf("IPAD:\n");
    for (size_t i = 0; i < 64; i++)
    {
        if(i < 63)
        {
            printf(" %02x", ipad[i]);
        }
        else
        {
            printf(" %02x\n", ipad[i]);
        }
    }

    printf("OPAD:\n");
    for (size_t i = 0; i < 64; i++)
    {
        if(i < 63)
        {
            printf(" %02x", opad[i]);
        }
        else
        {
            printf(" %02x\n", opad[i]);
        }
    }  
}

uint8_t *hmac(uint8_t *key, size_t key_length, uint8_t *msg, size_t msg_len)
{
    uint8_t *aux  = (uint8_t *)malloc((msg_len + 64) * sizeof(uint8_t));
    uint8_t *hash = (uint8_t *)malloc(sizeof(uint8_t) * 32);

    uint8_t t_ipad[64];
    uint8_t t_opad[64];

    update_pads(t_ipad, t_opad, key, key_length);

    memcpy(aux, t_ipad, 64);

    for(int i = 0; i < msg_len; i++)
    {
        aux[64 + i] = msg[i];
    }

    mbedtls_sha256(aux, msg_len + 64, hash, 0);

    for(int i = 0; i < 32; i++)
    {
        printf("%02x-", hash[i]);
    }
    printf("\n");

    free(aux);
    aux = (uint8_t *)malloc((msg_len + 64) * sizeof(uint8_t));

    memcpy(aux, t_opad, 64);
    
    for(int i = 0; i < 32; i++)
    {
        aux[64 + i] = hash[i];
    }
    
    mbedtls_sha256(aux, msg_len + 64, hash, 0);

    for(int i = 0; i < 32; i++)
    {
        printf("%02x-", hash[i]);
    }
    printf("\n");

    free(aux);
    return hash;
}