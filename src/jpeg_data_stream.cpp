#include "jpeg_data_stream.hpp"

uint8_t JpegDataStream::readBit(bool scan) {
    if (bitPos == 8) {
        bitPos = 0;
        currentByte = file.get();

        if (scan) {
            while (currentByte == MARKERSTART) {
                uint8_t currentMarker = file.peek();
                if (currentMarker == MARKERSTART) {
                    file.get(); // 0xFFFF00.. -> 0x00.., 0xFF in currentByte, 0x00 in currentMarker
                    currentMarker = file.peek();
                } else if (!currentMarker) {
                    file.get(); // 0xFF00AA.. -> 0xAA.., 0xFF in currentByte
                    break;

                } else if (currentMarker >= RST0 && currentMarker <= RST7) {
                    file.get(); // 0xFFD1FF00.. -> 0x00.., 0xFF in currentByte
                    currentByte = file.get();
                } else {
                    std::cerr << "Unexpected marker during readBit scan" << std::endl;
                    BYTE_TO_HEX(currentMarker);
                    std::cerr << std::endl;
                    break;
                }
            }
        }
    }

    uint8_t byte = (currentByte >> (7 - bitPos)) & 1;
    bitPos++;
    return byte;
}

uint64_t JpegDataStream::readBits(uint8_t length, bool scan) {

    uint64_t u1 = 0;
    for (uint8_t i = 0; i < length; i++) {
        uint8_t b1 = readBit(scan);
        if (b1 == 0xFF) {
            return -1;
        }
        u1 = (u1 << 1) | b1;
    }
    return u1;
}

uint8_t JpegDataStream::readByte(bool scan) {
    return readBits(8, scan);
}

uint8_t JpegDataStream::peekBit() const {
    return (currentByte >> (7 - bitPos)) & 1;
}
void JpegDataStream::align() {
    if (bitPos != 8) {
        bitPos = 8; // next readBit will read first bit of new byte
    }
}
bool JpegDataStream::isEOF() {
    return (file.peek() == EOF || file.eof() || file.fail());
}