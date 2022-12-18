#include "Uart.hpp"
#include <map>
#include <string.h>

#define DEFAULT_UART_BUFFER_SIZE    1024

Uart::Uart(const uart_port_t port, const eBaudRate baud, const uart_word_length_t wordLength, const uart_parity_t parity,
        const uart_stop_bits_t stopBits, const uart_hw_flowcontrol_t flowCtrl)
    :   _port{port}, _baud(baud), _wordLength(wordLength), _parity(parity), _stopBits(stopBits), _flowCtrl(flowCtrl) {

    uart_config_t uart_config = {
        .baud_rate = static_cast<int>(baud),
        .data_bits = wordLength,
        .parity = parity,
        .stop_bits = stopBits,
        .flow_ctrl = flowCtrl,
    };
    ESP_ERROR_CHECK(uart_driver_install(port, DEFAULT_UART_BUFFER_SIZE, DEFAULT_UART_BUFFER_SIZE, 0, nullptr, 0));
    ESP_ERROR_CHECK(uart_param_config(port, &uart_config));
}

char Uart::readChar(const size_t timeout) {
    char symb{};
    uart_read_bytes(_port, (uint8_t*)&symb, 1, timeout/portTICK_RATE_MS);
    return symb;
}

int Uart::write(const std::string& data) {
    return uart_write_bytes(_port, data.c_str(), data.length());
}

int Uart::write(const char* data) {
    return uart_write_bytes(_port, data, strlen(data));
}

int Uart::write(const uint8_t* data, const size_t length) {
    return uart_write_bytes(_port, (char*)data, length);
}

std::string Uart::read(const size_t timeout) {
    static const size_t maxReadLength{1024};
    std::string data{};
    data.reserve(maxReadLength);
    uart_read_bytes(_port, (uint8_t*)data.data(), maxReadLength, timeout/portTICK_RATE_MS);
    return data;
}