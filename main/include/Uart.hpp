#pragma once

#include "driver/uart.h"
#include <string>


enum class eBaudRate {
    BAUD_9600 = 9600,
    BAUD_19200 = 19200,
    BAUD_38400 = 38400,
    BAUD_56000 = 56000,
    BAUD_115000 = 115200,
    BAUD_256000 = 256000,
    BAUD_MAX,
};

class Uart {
    public:
        explicit Uart(const uart_port_t port, const eBaudRate, const uart_word_length_t, const uart_parity_t,
            const uart_stop_bits_t, const uart_hw_flowcontrol_t);
        int write(const std::string& data);
        int write(const char* data);
        int write(const uint8_t*, const size_t length);
        std::string read(const size_t timeout);
        char readChar(const size_t timeout);
    private:
        uart_port_t _port;
        eBaudRate _baud;
        uart_word_length_t _wordLength;
        uart_parity_t _parity;
        uart_stop_bits_t _stopBits;
        uart_hw_flowcontrol_t _flowCtrl;
};