#pragma once

#include "decode_huffman.hpp"
#include "jpeg_body.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <unordered_map>

bool decodeHuffman(std::unique_ptr<Body>& body);