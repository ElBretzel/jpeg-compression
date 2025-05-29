#include "jpeg_def.hpp"

// https://en.wikipedia.org/wiki/JPEG_File_Interchange_Format
// https://www.geocities.ws/crestwoodsdd/JPEG.htm
// https://help.accusoft.com/ImageGear-Net/v24.12/Windows/HTML/JPEG_Non-Image_Data_Structure.html

const uint8_t MARKERSTART = 0xFF;
const uint8_t SOI = 0xD8;
const uint8_t APP0 = 0xE0;
const uint8_t APPE = 0xEE;
const uint8_t APPF = 0xEF;
const uint8_t DQT = 0xDB;
const uint8_t DQTMI = 0x03;
const uint8_t DQTMP = 0x01;
const uint8_t DQTVAL = 0x40;
const uint8_t DQTBLOC = 0x08;
const uint8_t SOF0 = 0xC0;
const uint8_t SOF0BAS = 0x00;
const uint8_t SOF0PRE = 0x08;
const uint8_t SOF0MAXCOMP = 0x04;
const uint8_t SOF0LEN = 0x03;
const uint8_t SOF0MINSAMP = 0x01;
const uint8_t SOF0MAXSAMP = 0x04;
const uint8_t DHT = 0xC4;
const uint8_t DHTMHT = 0x01;
const uint8_t DHTBITS = 0x10;
const uint8_t SOS = 0xDA;
const uint8_t DRI = 0xDD;
const uint16_t DRILEN = 0x0004;
const uint8_t EOI = 0xD9;

// See ITU-T81 Fig A.6
const uint8_t zigZagMap[] = {0,  1,  5,  6,  14, 15, 27, 28, 2,  4,  7,  13, 16, 26, 29, 42, 3,  8,  12, 17, 25, 30,
                             41, 43, 9,  11, 18, 24, 31, 40, 44, 53, 10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38,
                             46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63};
const uint8_t reverseZigZagMap[] = {0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,
                                    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6,  7,  14, 21, 28,
                                    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
                                    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};