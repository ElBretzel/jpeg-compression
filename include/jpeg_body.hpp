#pragma once

#include "jpeg_data_stream.hpp"
#include "jpeg_header.hpp"

using MCUComponents = std::array<int16_t, 64>;
using MCUData = std::array<MCUComponents, 4>;

struct MCU {
    std::size_t mcuWidth;
    std::size_t mcuHeight;
    std::vector<MCUData> mcuData;
    const std::uint8_t size = 8;
};

struct Body {
    bool isValid = false;
    std::unique_ptr<Header> header;
    JpegDataStream data;
    std::unique_ptr<MCU> mcu;
};

std::unique_ptr<Body> scanBody(std::ifstream& jpegFile, std::unique_ptr<Header>& header);