#pragma once

#include "jpeg_def.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

struct Quantization {
    uint8_t tableId;
    uint8_t tablePrecision;
    bool completed;
    std::array<uint16_t, 64> values;
};

struct Channel {
    uint8_t verticalSampling;
    uint8_t horizontalSampling;
    uint8_t quantizationId;
    bool completed;
};

struct Header {
    bool isValid;
    uint8_t type = 0x00; // Baseline
    uint16_t width;
    uint16_t height;
    std::array<Quantization, 4> tables;
    std::array<Channel, 3> channels;
};

std::unique_ptr<Header> scanHeader(const std::string& filePath);
void printHeader(const Header& header);