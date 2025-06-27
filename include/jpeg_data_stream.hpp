#pragma once

#include <iostream>
#include <vector>

#include "jpeg_def.hpp"

struct Cursor {
    uint8_t bitPos = 0;
    std::size_t bytePos = 0;
};

class JpegDataStream {
  public:
    JpegDataStream() {}

    uint8_t readBit();
    uint8_t readByte();
    uint16_t readBits(uint8_t length);
    uint8_t peekBit() const;
    uint8_t peekByte() const;
    void align();
    void addByte(uint8_t b);
    uint8_t checkMarker() const;
    const Cursor& getCursor() const;
    void setCursor(Cursor& c);

  private:
    std::vector<uint8_t> data;
    Cursor cursor;
};