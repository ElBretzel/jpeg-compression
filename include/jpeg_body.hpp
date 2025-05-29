#pragma once

#include "jpeg_header.hpp"

struct Body {
    bool isValid = false;
    std::unique_ptr<Header> header;
    std::vector<uint8_t> data;
};

std::unique_ptr<Body> scanBody(std::ifstream& jpegFile, std::unique_ptr<Header>& header);