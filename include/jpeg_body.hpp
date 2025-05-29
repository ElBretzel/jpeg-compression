#pragma once

#include "jpeg_header.hpp"

struct Body {
    std::unique_ptr<Header> header;
    std::vector<uint8_t> data;
};

std::unique_ptr<Body> scanBody(const std::string& filePath, std::unique_ptr<Header>& header);