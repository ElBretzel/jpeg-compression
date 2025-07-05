#pragma once

#include "color_conversion.hpp"
#include "dct.hpp"
#include "quantization.hpp"

void writePPM(std::unique_ptr<Body>& image, const std::string& filename);
void dataToImage(std::unique_ptr<Body>& body);