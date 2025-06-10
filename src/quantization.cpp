#include "quantization.hpp"

void dequantize(std::unique_ptr<Body>& body) {
    for (auto& mcuData : body->mcu->mcuData) {
        for (uint8_t comp = 0; comp < body->header->numberComponents; ++comp) {
            const auto& quantTable = body->header->quantTable[body->header->channels[comp].quantizationId];
            if (!quantTable.completed) {
                std::cerr << "Dequantization error: Quantization table not completed for component "
                          << static_cast<int>(comp) << std::endl;
                return;
            }

            for (uint8_t i = 0; i < body->mcu->size * body->mcu->size; ++i) {
                mcuData[comp][i] *= quantTable.values[i];
            }
        }
    }
}
