#pragma once

#include "jpeg_body.hpp"
#include <unordered_map>

// key is {len-1, code} value is symbol
// this structure may take lot of memory space
using HuffmanDecodeTable = std::array<std::unordered_map<uint16_t, uint8_t>, 16>;

bool decodeHuffman(std::unique_ptr<Body>& body);