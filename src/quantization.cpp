#include "quantization.hpp"

void dequantize(std::unique_ptr<Body>& body) {
    for (auto& mcuData : body->mcu->mcuData) {
        for (uint8_t comp = 0; comp < body->header->numberComponents; ++comp) {
            const auto& quantTable = body->header->quantTable[body->header->channels[comp].quantizationId];
            for (uint8_t i = 0; i < body->mcu->size * body->mcu->size; ++i) {
                auto r = mcuData[comp][i];
                mcuData[comp][i] *= quantTable.values[i];
                r = mcuData[comp][i];
            }
        }
    }
}
