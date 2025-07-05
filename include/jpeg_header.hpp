#pragma once

#include "decode_huffman.hpp"
#include "jpeg_data_stream.hpp"
#include <memory>

struct Header {
    bool isValid;
    uint8_t type = 0xFF;      // Default value to track if type has been changed
    uint8_t component_offset; // According to ITU-T81, component ID can go from 0 to 255, if first componentID is non
                              // zero, we will take the offset
    uint16_t width;
    uint16_t height;
    uint8_t precision;
    uint8_t numberComponents; // 1 for grayscale, 3 for YCbCr, 4 for CMYK / YCbCrK (complience)
    uint16_t restartInterval;
    Progressive progressiveInfo;
    std::array<Quantization, 4> quantTable;
    std::array<Channel, 4> channels;          // Extra channels may be empty
    std::array<HuffmanTable, 8> huffmanTable; // Lot will be empty of progressive not used fully
    std::array<HuffmanDecodeTable, 8>
        decodeTables; // Ugly but lot of refactoring needs to be done to move inside HuffmanTable
};

std::unique_ptr<Header> scanHeader(JpegDataStream& jpegStream);
void printHeader(const Header& header);
bool fillSOS(JpegDataStream& jpegStream, std::unique_ptr<Header>& header);
bool fillDHT(JpegDataStream& jpegStream, std::unique_ptr<Header>& header);
bool fillRestart(JpegDataStream& jpegStream, std::unique_ptr<Header>& header);
bool fillDQT(JpegDataStream& jpegStream, std::array<Quantization, 4>& tables);
bool fillCom(JpegDataStream& jpegStream, std::unique_ptr<Header>& header);
bool fillApp(JpegDataStream& jpegStream, std::unique_ptr<Header>& header);
void printSOSTable(const Header& header);
void printDQTTable(const Header& header);
void printSOFTable(const Header& header);
void printDRITable(const Header& header);
void printDHTTable(const Header& header);