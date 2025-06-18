#include "dct.hpp"

// See ITU-T81 A.3.3
int16_t IDCT(MCUComponents& S, uint8_t mcuSize, uint8_t y, uint8_t x) {
    double s = 0;
    double cu, cv;

    for (uint8_t u = 0; u < mcuSize; u++) {
        for (uint8_t v = 0; v < mcuSize; v++) {
            if (u == 0) {
                cu = 1.0 / sqrt(2.0);
            } else {
                cu = 1.0;
            }
            if (v == 0) {
                cv = 1.0 / sqrt(2.0);
            } else {
                cv = 1.0;
            }
            s += cu * cv * S[u * mcuSize + v] * cos((2.0 * x + 1.0) * v * M_PI / (2.0 * mcuSize)) *
                 cos((2.0 * y + 1.0) * u * M_PI / (2.0 * mcuSize));
        }
    }

    // 8bits is hardcoded
    return static_cast<int16_t>(std::floor(s * 0.25) + 128);
}

void IDCT(MCUComponents& data, uint8_t mcuSize) {
    MCUComponents tempData = data;
    for (uint8_t y = 0; y < mcuSize; y++) {
        for (uint8_t x = 0; x < mcuSize; x++) {
            data[y * mcuSize + x] = IDCT(tempData, mcuSize, y, x);
        }
    }
}

void inverseDCT(std::unique_ptr<Body>& body) {
    for (auto& mcuData : body->mcu->mcuData) {
        for (uint8_t comp = 0; comp < body->header->numberComponents; ++comp) {
            IDCT(mcuData[comp], body->mcu->size);
        }
    }
}