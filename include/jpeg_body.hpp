#pragma once

#include "jpeg_data_stream.hpp"
#include "jpeg_header.hpp"

struct Body {
    bool isValid = false;
    std::unique_ptr<Header> header;
    JpegDataStream data;
};

std::unique_ptr<Body> scanBody(std::ifstream& jpegFile, std::unique_ptr<Header>& header);