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
extern const uint8_t APPE; // Photoshop (and Adobe) may influence color channel id mapping
extern const uint8_t APPF;
extern const uint8_t DQT;
extern const uint8_t DQTMP;
extern const uint8_t DQTMI;
extern const uint8_t DQTVAL;
extern const uint8_t DQTBLOC;
extern const uint8_t SOF0;
extern const uint8_t SOF0BAS;
extern const uint8_t SOF0PRE;
extern const uint8_t SOF0MAXCOMP;
extern const uint8_t SOF0MINSAMP;
extern const uint8_t SOF0MAXSAMP;
extern const uint8_t DHT;
extern const uint8_t DHTMHT;
extern const uint8_t DHTBITS;
extern const uint8_t SOS;
extern const uint8_t SOSSPECS;
extern const uint8_t SOSSPECE;
extern const uint8_t SOSSUCC;
extern const uint8_t DRI;
extern const uint16_t DRILEN;
extern const uint8_t EOI;

extern const uint8_t zigZagMap[];
extern const uint8_t reverseZigZagMap[];