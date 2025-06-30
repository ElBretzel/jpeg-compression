#include "color_conversion.hpp"
#include "dct.hpp"
#include "huffman_code.hpp"
#include "jpeg_scan.hpp"
#include "quantization.hpp"

void writePPM(std::unique_ptr<Body>& image, const std::string& filename) {
    if (!image || !image->header || !image->mcu || !image->isValid || !image->mcu->isValid) {
        std::cout << "Error - Invalid image data\n";
        return;
    }

    std::cout << "Writing " << filename << "...\n";

    std::ofstream outFile(filename, std::ios::out | std::ios::binary);
    if (!outFile.is_open()) {
        std::cout << "Error - Could not open output file\n";
        return;
    }

    const uint16_t width = image->header->width;
    const uint16_t height = image->header->height;

    // PPM header
    outFile << "P6\n" << width << " " << height << "\n255\n";
    std::vector<uint8_t> buffer(width * height * 3);

    for (uint y = 0; y < height; ++y) {
        const uint mcuRow = y / 8;
        const uint pixelRow = y % 8;
        for (uint x = 0; x < width; ++x) {
            const uint mcuColumn = x / 8;
            const uint pixelColumn = x % 8;
            const uint mcuIndex = mcuRow * image->mcu->mcuWidth + mcuColumn;
            const uint pixelIndex = pixelRow * 8 + pixelColumn;

            // Assuming MCUData[0] = Y, [1] = Cb, [2] = Cr and [3] = RGB (after color conversion)
            uint8_t r, g, b;
            switch (image->header->numberComponents) {
            case 1:
                r = image->mcu->mcuData[mcuIndex][0][pixelIndex];
                g = image->mcu->mcuData[mcuIndex][0][pixelIndex];
                b = image->mcu->mcuData[mcuIndex][0][pixelIndex];
                break;
            case 3:
                r = image->mcu->mcuData[mcuIndex][0][pixelIndex];
                g = image->mcu->mcuData[mcuIndex][1][pixelIndex];
                b = image->mcu->mcuData[mcuIndex][2][pixelIndex];
                break;
            default:
                std::cerr << "Only 1 or 3 channels supported" << std::abort;
                return;
            }

            size_t index = (y * width + x) * 3;
            buffer[index] = r;
            buffer[index + 1] = g;
            buffer[index + 2] = b;
        }
    }

    outFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    outFile.close();
    std::cout << "PPM file written successfully.\n";
}

void prelude() {
    JpegDataStream jpegStream = JpegDataStream("/home/dluca/Documents/Epita/TIFO/jpeg-compression/demo/prog/earth.jpg");

    auto header = scanHeader(jpegStream);
    auto body = fillScans(jpegStream, header);
    header = nullptr; // Ownership moved into body
    dequantize(body);
    inverseDCT(body);
    convertMCUToRGB(body);

    printHeader(*body->header);
    std::cout << "Body valid: " << static_cast<bool>(body->isValid) << std::endl;
    std::cout << "MCU valid: " << static_cast<bool>(body->mcu->isValid) << std::endl;
    writePPM(body, "/home/dluca/Documents/Epita/TIFO/jpeg-compression/demo/out.ppm");
}

int main() {
    prelude();
    return EXIT_SUCCESS;
}
