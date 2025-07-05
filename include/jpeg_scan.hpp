#pragma once

#include "huffman_code.hpp"

std::unique_ptr<Body> fillScans(JpegDataStream& jpegStream, std::unique_ptr<Header>& header);