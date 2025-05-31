#include "file_browser.hpp"
#include "frame_counter.hpp"
#include "sprite_handler.hpp"

#include "decode_huffman.hpp"
#include "jpeg_body.hpp"
#include "jpeg_header.hpp"

FileBrowser fileBrowser;

// This function is entered each frame
void process(const sf::Time& delta, sf::RenderWindow& window) {
    FrameCounter frameCounter(delta);
    frameCounter.move({10, window.getSize().y - 40});
    frameCounter.process();

    fileBrowser.move({10, 10});
    fileBrowser.process();
}

void initBrowser() {
    fileBrowser.setTitle("File Browser");
    fileBrowser.setFlags(ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                         ImGuiWindowFlags_NoMove);
    fileBrowser.move({10, 10});
    fileBrowser.resize({60, 20});
    fileBrowser.setFilters({".ppm", ".bmp", ".jpg"});
}

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
            const uint8_t r = image->mcu->mcuData[mcuIndex][1][pixelIndex];
            const uint8_t g = image->mcu->mcuData[mcuIndex][2][pixelIndex];
            const uint8_t b = image->mcu->mcuData[mcuIndex][3][pixelIndex];

            outFile.put(r);
            outFile.put(g);
            outFile.put(b);
        }
    }

    outFile.close();
    std::cout << "PPM file written successfully.\n";
}

void prelude() {
    auto filePath = std::string("/home/dluca/Documents/Epita/TIFO/jpeg-compression/demo/cat.jpg");
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
    // Dequantize
    // IDCT
    // IRGB
    // Profit

    printHeader(*body->header);
    std::cout << "Body valid: " << static_cast<bool>(body->isValid) << std::endl;
    std::cout << "MCU valid: " << static_cast<bool>(body->mcu->isValid) << std::endl;
    writePPM(body, "/home/dluca/Documents/Epita/TIFO/jpeg-compression/demo/out.ppm");
}

int main() {
    prelude();

    return EXIT_SUCCESS;

    sf::RenderWindow window(sf::VideoMode({1280, 720}), "JPEG Implementation");
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window))
        return EXIT_FAILURE;

    initBrowser();
    sf::Clock deltaClock;

    while (window.isOpen()) {

        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        const sf::Time delta = deltaClock.restart();
        ImGui::SFML::Update(window, delta);

        process(delta, window);

        window.clear();
        ImGui::ShowDemoWindow();

        if (fileBrowser.fileAvailable) {
            if (addSprite(fileBrowser.getSelectedFile(), {200, 200})) {
                break;
            }
        }

        for (const auto& [sp_container, sp] : spriteMap) {
            window.draw(*sp);
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return EXIT_SUCCESS;
}
