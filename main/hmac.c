#include <string.h>
#include <mbedtls/sha256.h>
#include <freertos/FreeRTOS.h>
#include "hmac.h"

// void *fmemcpy(void *dst, const void *src, size_t num_bytes_to_copy)
// {
//     uint8_t *d = (uint8_t *)dst;
//     uint8_t *s = (uint8_t *) s;

//     while (num_bytes_to_copy--)
//     {
//         *d++ = *s++;
//     }
    
// }

// uint8_t *build_msg(const uint8_t *msg, size_t msg_len, size_t formmated_msg_len, bool is_litte_endian)
// {
//     uint8_t *fmt_msg;
//     size_t cursor;
//     if(!(fmt_msg = malloc(formmated_msg_len)))
//     {
//         return NULL;
//     }
//     memcpy(fmt_msg, msg, msg_len);
//     fmt_msg[formmated_msg_len] = 0b10000000;
//     cursor = msg_len + 1;

//     while (cursor < formmated_msg_len)
//     {
//         fmt_msg[cursor++] = 0;
//     }
    
//     (uint64_t   )
// }

void hmac(uint8_t *key, size_t key_length, uint8_t *msg, size_t msg_len, uint8_t *hmac_buffer)
{
    uint8_t *aux  = (uint8_t *)malloc((msg_len + 64) * sizeof(uint8_t));

    uint8_t t_ipad[64];
    uint8_t t_opad[64];

    for(int i = 0; i < 64; i++)
    {
        t_ipad[i] = IPAD_VALUE;
        t_opad[i] = OPAD_VALUE;
    }

    for(int i = 0; i < key_length; i++)
    {
        t_ipad[i] ^= key[i];
        t_opad[i] ^= key[i];
    }

    memcpy(aux, t_ipad, 64);

    for(int i = 0; i < msg_len; i++)
    {
        aux[64 + i] = msg[i];
    }

    mbedtls_sha256(aux, msg_len + 64, hmac_buffer, 0);

    free(aux);
    aux = (uint8_t *)malloc((msg_len + 64) * sizeof(uint8_t));

    memcpy(aux, t_opad, 64);
    
    for(int i = 0; i < 32; i++)
    {
        aux[64 + i] = hmac_buffer[i];
    }

    mbedtls_sha256(aux, msg_len + 64, hmac_buffer, 0);

    free(aux);
}