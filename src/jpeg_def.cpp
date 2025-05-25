#include "jpeg_def.hpp"

// https://en.wikipedia.org/wiki/JPEG_File_Interchange_Format
// https://www.geocities.ws/crestwoodsdd/JPEG.htm
// https://help.accusoft.com/ImageGear-Net/v24.12/Windows/HTML/JPEG_Non-Image_Data_Structure.html

const uint16_t SOI = 0xFFD8;
const uint8_t APPN = 0xFF;
const uint8_t APP0 = 0xE0;
const uint8_t APP1 = 0xE1;
const uint8_t APP2 = 0xE2;
const uint8_t APP3 = 0xE3;
const uint8_t APP4 = 0xE4;
const uint8_t APP5 = 0xE5;
const uint8_t APP6 = 0xE6;
const uint8_t APP7 = 0xE7;
const uint8_t APP8 = 0xE8;
const uint8_t APP9 = 0xE9;
const uint8_t APPA = 0xEA;
const uint8_t APPB = 0xEB;
const uint8_t APPC = 0xEC;
const uint8_t APPD = 0xED;
const uint8_t APPE = 0xEE;
const uint8_t APPF = 0xEF;
const uint16_t DQT = 0xFFDB;
const uint8_t DQTMI = 0x03;
const uint8_t DQTMP = 0x01;
const uint8_t DQTVAL = 0x40;
const uint8_t DQTBLOC = 0x08;
const uint16_t SOF0 = 0xFFC0;
const uint16_t SOS = 0xFFDA;
const uint16_t EOI = 0xFFD9;

// https://www.researchgate.net/profile/Fernando-Martin-Rodriguez/publication/369759372/figure/fig2/AS:11431281136821988@1680579134024/Zig-zag-ordering-from-JPEG-standard.png
const uint8_t zigZagMap[] = {0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,  12, 19, 26, 33, 40, 48,
                             41, 34, 27, 20, 13, 6,  7,  14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23,
                             30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};