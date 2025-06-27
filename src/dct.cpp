#include "dct.hpp"

static std::vector<std::vector<float>> cosLUT;
static std::vector<float> factors;
static bool precomputed = false;
static bool precomputedSize = 8;

void precomputeLUT(uint8_t mcuSize) {
    if (precomputed && mcuSize == precomputedSize) {
        return;
    }
    factors.resize(mcuSize);
    cosLUT.resize(mcuSize);
    for (auto& rows : cosLUT) {
        rows.resize(mcuSize);
    }

    factors[0] = 1.0f / sqrtf(2.0f);
    for (auto i = 1; i < mcuSize; i++) {
        factors[i] = 1.0f;
    }

    for (auto coord = 0; coord < mcuSize; coord++) {
        for (auto sub = 0; sub < mcuSize; sub++) {
            cosLUT[coord][sub] = cosf((2.0f * coord + 1.0f) * sub * M_PI / (2.0f * mcuSize));
        }
    }

    precomputed = true;
    precomputedSize = mcuSize;
}

// See ITU-T81 A.3.3
int16_t IDCT(MCUComponents& S, uint8_t mcuSize, uint8_t y, uint8_t x) {
    float s = 0;

    for (uint8_t u = 0; u < mcuSize; u++) {
        float cu = factors[u];
        float cosCu = cosLUT[y][u];
        int offset = u * mcuSize;
        for (uint8_t v = 0; v < mcuSize; v++) {
            float cv = factors[v];
            float cosCv = cosLUT[x][v];
            s += cu * cv * S[offset + v] * cosCu * cosCv;
        }
    }

    // 8bits is hardcoded
    return static_cast<int16_t>(std::floorf(s * 0.25f) + 128.0f);
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
    precomputeLUT(body->mcu->size);
    for (auto& mcuData : body->mcu->mcuData) {
        for (uint8_t comp = 0; comp < body->header->numberComponents; ++comp) {
            IDCT(mcuData[comp], body->mcu->size);
        }
    }
}