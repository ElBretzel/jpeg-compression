#pragma once

#include <cstdint>

#define HEX_DIGIT(n) ((n) < 10 ? '0' + (n) : 'A' + ((n) - 10))

#define BYTE_TO_HEX(byte)                                                                                              \
    putchar(HEX_DIGIT(((byte) >> 4) & 0xF));                                                                           \
    putchar(HEX_DIGIT((byte) & 0xF));                                                                                  \
    putchar(' ');

constexpr uint8_t MARKERSTART = 0xFF;
constexpr uint8_t SOI = 0xD8;
constexpr uint8_t APP0 = 0xE0;
constexpr uint8_t APP1 = 0xE1;
constexpr uint8_t APP2 = 0xE2;
constexpr uint8_t APP3 = 0xE3;
constexpr uint8_t APP4 = 0xE4;
constexpr uint8_t APP5 = 0xE5;
constexpr uint8_t APP6 = 0xE6;
constexpr uint8_t APP7 = 0xE7;
constexpr uint8_t APP8 = 0xE8;
constexpr uint8_t APP9 = 0xE9;
constexpr uint8_t APPA = 0xEA;
constexpr uint8_t APPB = 0xEB;
constexpr uint8_t APPC = 0xEC;
constexpr uint8_t APPD = 0xED;
constexpr uint8_t APPE = 0xEE;
constexpr uint8_t APPF = 0xEF;
constexpr uint8_t DQT = 0xDB;
constexpr uint8_t DQTMI = 0x03;
constexpr uint8_t DQTMP = 0x01;
constexpr uint8_t MCUX = 0x40;
constexpr uint8_t DQTBLOC = 0x08;
constexpr uint8_t SOF0 = 0xC0;
constexpr uint8_t SOF2 = 0xC2;
constexpr uint8_t SOF0BAS = 0x00;
constexpr uint8_t SOF0PRE = 0x08;
constexpr uint8_t SOF2MINPRE = 0x08;
constexpr uint8_t SOF2MAXPRE = 0x0C;
constexpr uint8_t SOFMAXCOMP = 0x04;
constexpr uint8_t SOFMINSAMP = 0x01;
constexpr uint8_t SOFMAXSAMP = 0x04;
constexpr uint8_t DHT = 0xC4;
constexpr uint8_t DHTMHT = 0x01;
constexpr uint8_t DHTBITS = 0x10;
constexpr uint8_t SOS = 0xDA;
constexpr uint8_t SOSSUCCBITMAX = 0x0D;
constexpr uint8_t EOS = 0x3F;
constexpr uint8_t DRI = 0xDD;
constexpr uint8_t RST0 = 0xD0;
constexpr uint8_t RST7 = 0xD7;
constexpr uint16_t DRILEN = 0x0004;
constexpr uint8_t EOI = 0xD9;
constexpr uint8_t EOB = 0x00;
constexpr uint8_t ZLR = 0x0F;

// See ITU-T81 Fig A.6
constexpr uint8_t zigZagMap[] = {0,  1,  5,  6,  14, 15, 27, 28, 2,  4,  7,  13, 16, 26, 29, 42, 3,  8,  12, 17, 25, 30,
                                 41, 43, 9,  11, 18, 24, 31, 40, 44, 53, 10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38,
                                 46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63};
constexpr uint8_t reverseZigZagMap[] = {0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,
                                        12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6,  7,  14, 21, 28,
                                        35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
                                        58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};