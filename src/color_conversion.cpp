#include "color_conversion.hpp"

RGB YCbCrToRGB(const YCbCr& comp) {
    RGB rgb;

    rgb[0] = static_cast<uint8_t>(std::clamp(comp[0] + 1.402 * (comp[2] - 128.0), 0.0, 255.0)); // R
    rgb[1] = static_cast<uint8_t>(
        std::clamp(comp[0] - 0.344136 * (comp[1] - 128.0) - 0.714136 * (comp[2] - 128.0), 0.0, 255.0)); // G
    rgb[2] = static_cast<uint8_t>(std::clamp(comp[0] + 1.772 * (comp[1] - 128.0), 0.0, 255.0));         // B

    return rgb;
}

void convertMCUToRGB(std::unique_ptr<Body>& body) {
    for (auto& mcu : body->mcu->mcuData) {
        for (uint8_t i = 0; i < body->mcu->size * body->mcu->size; i++) {
            YCbCr ycbcr;
            if (body->header->numberComponents == 1) {
                ycbcr = {mcu[0][i], mcu[0][i], mcu[0][i]};
            } else if (body->header->numberComponents == 3) {
                ycbcr = {mcu[0][i], mcu[1][i], mcu[2][i]};
            } else {
                std::cerr << "Conversion to RGB is only supported for 1 or 3 components" << std::endl;
                return;
            }
            RGB rgb = YCbCrToRGB(ycbcr);
            mcu[0][i] = rgb[0];
            mcu[1][i] = rgb[1];
            mcu[2][i] = rgb[2];
        }
    }
}