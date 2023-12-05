#ifndef UART_H
#define UART_H

#include <cstdint>
uint32_t uart_read();
uint32_t uart_state();
void uart_b1_set_ovf(uint32_t ovf);
void uart_b2_config(uint32_t config);
void uart_b3_tx(uint32_t bus);

constexpr uint32_t uart_state_offset = 5;
constexpr uint32_t uart_tx_busy_bit = 1;
constexpr uint32_t uart_rx_ready_bit = 2;

#endif
