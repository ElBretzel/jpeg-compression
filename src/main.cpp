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

void prelude() {
    auto filePath = std::string("/home/dluca/Documents/Epita/TIFO/jpeg-compression/demo/4.1.04.jpg");
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

    // printHeader(*body->header);
    // std::cout << "Body valid: " << static_cast<bool>(body->isValid) << std::endl;
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
