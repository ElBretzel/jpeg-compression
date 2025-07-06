#include "jpeg_transform.hpp"
#include <filesystem>

static int n = 0;

void writePPM(std::unique_ptr<Body>& image) {
    const std::string& jpegPath = image->data.filePath;

    std::filesystem::path inputPath(jpegPath);
    std::string filename = inputPath.stem().string(); // e.g., "photo"
    std::string outDir = inputPath.parent_path().string() + "/output";

    std::filesystem::create_directories(outDir);

    std::string outFilePath = outDir + "/" + filename + "_" + std::to_string(n++) + ".ppm";
    std::cout << "Writing " << outFilePath << "...\n";

    std::ofstream outFile(outFilePath, std::ios::out | std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error - Could not open output file\n";
        return;
    }

    const uint16_t width = image->header->width;
    const uint16_t height = image->header->height;

    outFile << "P6\n" << width << " " << height << "\n255\n";
    std::vector<uint8_t> buffer(width * height * 3);

    for (uint y = 0; y < height; ++y) {
        const uint mcuRow = y / 8;
        const uint pixelRow = y % 8;
        for (uint x = 0; x < width; ++x) {
            const uint mcuColumn = x / 8;
            const uint pixelColumn = x % 8;
            const uint mcuIndex = mcuRow * image->mcuDecoded->mcuWidth + mcuColumn;
            const uint pixelIndex = pixelRow * 8 + pixelColumn;

            uint8_t r, g, b;
            switch (image->header->numberComponents) {
            case 1:
                r = g = b = image->mcuDecoded->mcuData[mcuIndex][0][pixelIndex];
                break;
            case 3:
                r = image->mcuDecoded->mcuData[mcuIndex][0][pixelIndex];
                g = image->mcuDecoded->mcuData[mcuIndex][1][pixelIndex];
                b = image->mcuDecoded->mcuData[mcuIndex][2][pixelIndex];
                break;
            default:
                std::cerr << "Only 1 or 3 channels supported" << std::endl;
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

void dataToImage(std::unique_ptr<Body>& body) {
    body->mcuDecoded->mcuData.clear();
    for (auto& mcuData : body->mcu->mcuData) {
        body->mcuDecoded->mcuData.push_back(mcuData);
    }

    dequantize(body);
    inverseDCT(body);
    convertMCUToRGB(body);
    writePPM(body);
}
