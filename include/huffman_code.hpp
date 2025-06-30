#pragma once

#include "jpeg_body.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <unordered_map>

// rename jpeg_body.cpp / jpeg_body.hpp to jpeg_scan.cpp/hpp
// create new jpeg_body with struct

// key is {len-1, code} value is symbol
// this structure may take lot of memory space
using HuffmanDecodeTable = std::array<std::unordered_map<uint16_t, uint8_t>, 16>;

bool decodeHuffman(std::unique_ptr<Body>& body);