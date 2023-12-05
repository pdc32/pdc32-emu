#include "uart.h"
#include <iostream>

uint8_t uart_data_bits = 8;

uint16_t uart_read_data = 0;
bool uart_has_read = false;

uint32_t uart_read() {
    // Tx is immediate so we don't set Tx flag
    if(uart_has_read) {
        return uart_rx_ready_bit;
    }
    return 0;
}

uint32_t uart_state() {
    return uart_read_data;
}

void uart_set_data(uint16_t data) {
    uart_read_data = data;
    uart_has_read = true;
}

void uart_b1_set_ovf(uint32_t ovf) {
    auto speed = 24000000 / ovf;
    std::cout << "UART OFV: speed=" << speed << " bps (* might need more math)" << std::endl;
}

void uart_b2_config(uint32_t word) {
    bool parity_odd = (word >> 16) & 0b1;
    uint8_t databits = 6 + ((word >> 17) & 0b11);
    bool parity_enabled = (word >> 19) & 0b1;
    uint8_t stopbits = 1 + (word >> 20);

    uart_data_bits = databits;

    std::cout << "UART CONFIG:"
        << " databits=" << unsigned(databits)
        << " parity=" << (parity_enabled ? (parity_odd ? "ODD" : "EVEN") : "OFF")
        << " stopbits=" << unsigned(stopbits)
        << std::endl;
}

void uart_b3_tx(uint32_t bus) {
    uint16_t mask = (1 << (uart_data_bits + 1)) - 1; // lower bits
    uint16_t word = bus & mask;
    char data[5]; // in case uart_data_bits gets corrupted to something invalid, the bus might be 4 hexa chars + NULL

    if (uart_data_bits <= 8) {
        data[0] = (char) bus & 0xFF;
        data[1] = '\0';
    } else {
        snprintf(data, 4, "%x", word);
    }

    std::cout << "UART TX: (" << unsigned(uart_data_bits) << " bits) " << (char *) data << std::endl;
}
