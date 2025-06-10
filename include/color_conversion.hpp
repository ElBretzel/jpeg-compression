#pragma once

#include "jpeg_body.hpp"

#include <algorithm>
#include <array>
#include <cstdint>

using RGB = std::array<uint8_t, 3>;
using YCbCr = std::array<int16_t, 3>;

// https://en.wikipedia.org/wiki/YCbCr#JPEG_conversion
RGB YCbCrToRGB(const YCbCr& comp);

void convertMCUToRGB(std::unique_ptr<Body>& body);
