#pragma once
#include <stdint.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <esp_mac.h>

#define BAUD_RATE 9600
#define TX_BUF 1024
#define RX_BUF 1024

void setup_uart(QueueHandle_t *uart_queue);
uint16_t crc16arc_bit(uint16_t crc, void const *mem, size_t len);
uint16_t replaceByte(uint16_t value, uint8_t b, uint16_t pos);

void set(uint8_t *data, uint8_t command, uint8_t *payload, uint8_t payload_len);
int send(uint8_t *s, uint8_t *r, uint16_t size_of_s, uint16_t size_of_r, uint8_t retries);
int send_solved_string(uint8_t *s, uint8_t *r, uint16_t size_of_s, uint16_t size_of_r, uint8_t retries);