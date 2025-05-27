#pragma once

#include "jpeg_def.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

struct Quantization {
    uint8_t tableId;        // between 0 and 3
    uint8_t tablePrecision; // 0 or 1
    bool completed;
    std::array<uint16_t, 64> values;
};

struct Channel {
    uint8_t horizontalSampling; // between 1 and 4
    uint8_t verticalSampling;   // between 1 and 4
    uint8_t quantizationId;     // between 0 and 3
    bool completed;
};

struct Header {
    bool isValid;
    uint8_t type = 0x00; // Baseline
    uint16_t width;
    uint16_t height;
    uint8_t numberComponents; // 1 for grayscale or 3 for RGB
    std::array<Quantization, 4> tables;
    std::array<Channel, 3> channels; // Extra channels may be empty
};

std::unique_ptr<Header> scanHeader(const std::string& filePath);
void printHeader(const Header& header);