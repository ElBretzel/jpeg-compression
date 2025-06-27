#include "color_conversion.hpp"
#include "dct.hpp"
#include "huffman_code.hpp"
#include "jpeg_body.hpp"
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

            outFile.put(r);
            outFile.put(g);
            outFile.put(b);
        }
    }

    outFile.close();
    std::cout << "PPM file written successfully.\n";
}

void prelude() {
    auto filePath = std::string("/home/dluca/Documents/Epita/TIFO/jpeg-compression/demo/turtle.jpg");
    if (filePath.empty()) {
        std::cerr << "Can not check validity of jpeg file: " << "path is empty" << std::endl;
        return;
    }
    if (!filePath.ends_with(".jpg") && !filePath.ends_with(".jpeg")) {
        std::cerr << "Can not check validity of jpeg file: " << "file has incorrect format" << std::endl;
        return;
    }

    std::ifstream jpegFile(filePath, std::ios::binary);

    auto header = scanHeader(jpegFile);
    auto body = scanBody(jpegFile, header);
    header = nullptr; // Ownership moved into body
    jpegFile.close();
    decodeHuffman(body);
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
