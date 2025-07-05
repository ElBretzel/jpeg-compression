#pragma once

#include "jpeg_def.hpp"
#include <iostream>
#include <unordered_map>

// key is {len-1, code} value is symbol
// this structure may take lot of memory space
using HuffmanDecodeTable = std::array<std::unordered_map<uint16_t, uint8_t>, 16>;

bool generateCode(HuffmanTable& tables);
bool fillDecodeTable(const HuffmanTable& table, HuffmanDecodeTable& decodeTable);