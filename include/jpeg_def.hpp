#pragma once

#include <cstdint>

#define HEX_DIGIT(n) ((n) < 10 ? '0' + (n) : 'A' + ((n) - 10))

#define BYTE_TO_HEX(byte)                                                                                              \
    putchar(HEX_DIGIT(((byte) >> 4) & 0xF));                                                                           \
    putchar(HEX_DIGIT((byte) & 0xF));                                                                                  \
    putchar(' ');

// https://en.wikipedia.org/wiki/JPEG_File_Interchange_Format
// https://www.geocities.ws/crestwoodsdd/JPEG.htm
// https://help.accusoft.com/ImageGear-Net/v24.12/Windows/HTML/JPEG_Non-Image_Data_Structure.html

extern const uint8_t MARKERSTART;
extern const uint8_t SOI;
extern const uint8_t APP0;
extern const uint8_t APP1;
extern const uint8_t APP2;
extern const uint8_t APP3;
extern const uint8_t APP4;
extern const uint8_t APP5;
extern const uint8_t APP6;
extern const uint8_t APP7;
extern const uint8_t APP8;
extern const uint8_t APP9;
extern const uint8_t APPA;
extern const uint8_t APPB;
extern const uint8_t APPC;
extern const uint8_t APPD;
extern const uint8_t APPE;
extern const uint8_t APPF;
extern const uint8_t DQT;
extern const uint8_t DQTMP;
extern const uint8_t DQTMI;
extern const uint8_t DQTVAL;
extern const uint8_t DQTBLOC;
extern const uint8_t SOF0;
extern const uint8_t SOS;
extern const uint8_t DHT;
extern const uint8_t EOI;
extern const uint8_t DNL;
extern const uint8_t DRI;
extern const uint8_t COM;

extern const uint8_t zigZagMap[];
extern const uint8_t reverseZigZagMap[];