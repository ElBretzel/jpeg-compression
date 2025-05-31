#include "jpeg_data_stream.hpp"

uint8_t JpegDataStream::readBit() {
    if (cursor.bytePos >= data.size()) {
        std::cerr << "jpegDataStream EOS" << std::endl;
        return 0xFF;
    }

    uint8_t b = peekBit();
    cursor.bitPos++;
    if (cursor.bitPos == 8) {
        cursor.bitPos = 0;
        cursor.bytePos++;
    }

    return b;
}
uint8_t JpegDataStream::readByte() {
    return readBits(8);
}
uint16_t JpegDataStream::readBits(uint8_t length) {
    if (length > DHTBITS) {
        std::cerr << "jpegDataStream can not read more than 16 bits" << std::endl;
        return 0xFFFF;
    }

    uint16_t u1 = 0;
    for (uint8_t i = 0; i < length; i++) {
        uint8_t b1 = readBit();
        if (b1 == 0xFF) {
            return 0xFFFF;
        }
        u1 = (u1 << 1) | b1;
    }
    return u1;
}
uint8_t JpegDataStream::peekBit() const {
    uint8_t b1 = data[cursor.bytePos];
    return (b1 >> (7 - cursor.bitPos)) & 1;
}
uint8_t JpegDataStream::peekByte() const {

    uint8_t bitPos = cursor.bitPos;
    std::size_t bytePos = cursor.bytePos;

    uint8_t u1 = 0;

    // Combine peekBit, readBit and readBits logic
    for (uint8_t i = 0; i < 8; i++) {
        if (bytePos >= data.size()) {
            return 0xFF;
        }
        uint8_t b1 = data[bytePos];
        uint8_t b2 = (b1 << (7 - bitPos)) & 1;

        u1 = (u1 << 1) | b2;
        bitPos++;
        if (bitPos = 8) {
            bitPos = 0;
            bytePos++;
        }
    }

    return u1;
}
void JpegDataStream::align() {
    if (cursor.bitPos != 0) {
        cursor.bitPos = 0;
        cursor.bytePos++;
    }
}
void JpegDataStream::addByte(uint8_t b) {
    data.push_back(b);
}

uint8_t JpegDataStream::checkMarker() const {
    if (cursor.bitPos != 0) {
        return 0xFF;
    }
    if (cursor.bytePos >= data.size()) {
        return 0xFF;
    }
    if (data[cursor.bytePos] != 0xFF) {
        return 0xFF;
    }
    if (cursor.bytePos + 1 >= data.size()) {
        return 0xFF;
    }
    return data[cursor.bytePos + 1];
}
const Cursor& JpegDataStream::getCursor() const {
    return cursor;
}
void JpegDataStream::setCursor(Cursor& c) {
    cursor.bitPos = c.bitPos;
    cursor.bytePos = c.bytePos;
}