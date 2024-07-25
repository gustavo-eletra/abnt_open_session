#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define IPAD_VALUE 0x36
#define OPAD_VALUE 0x5c

// uint8_t *build_msg(const uint8_t *msg, size_t length, size_t formmated_msg_len, bool is_litte_endian);
// void *fmemcpy(void *dst, const void *src, size_t n);

uint8_t *hmac(uint8_t *key, size_t key_length,uint8_t *msg, size_t msg_len);
void update_pads(uint8_t *ipad, uint8_t *opad, uint8_t *key, int key_len);