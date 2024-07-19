#pragma once

#include <stdint.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <esp_mac.h>

#include "CommandQueue.h"

#define BAUD_RATE 9600
#define TX_BUF 1024
#define RX_BUF 1024

// typedef enum
// {
//     OPEN_SESSION,
//     SEND_COMMAND,
// }COMMANDS;

typedef struct
{
    uint8_t *ds; //To send
    uint8_t *dr; //To receive
    int ds_size;
    int dr_size;
} UARTData;

extern CommandQueue cq;
extern UARTData uart_data;

void setup_uart();
void open_session();
void send_command();
uint8_t *get_uart_rx_byte_array(CommandQueue *cq, int command);
int uart_send(void *data);
void open_abnt_session(void *data);
uint16_t crc16arc_bit(uint16_t crc, void const *mem, size_t len);
uint16_t replaceByte(uint16_t value, uint8_t b, uint16_t pos);
UARTData *uart_data_as_abnt();
void uart_data_as_dlms(UARTData *data);

void set_abnt_data(UARTData *data, uint8_t command, uint8_t *payload, uint8_t payload_len);