#pragma once

#include "huffman_code.hpp"
#include "jpeg_transform.hpp"

std::unique_ptr<Body> fillScans(JpegDataStream& jpegStream, std::unique_ptr<Header>& header);
bool scanMarker(JpegDataStream& jpegStream, std::unique_ptr<Body>& body, uint8_t type);