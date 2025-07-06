#include "huffman_code.hpp"
#include "jpeg_scan.hpp"
#include "jpeg_transform.hpp"
#include <filesystem>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <jpeg_file_path>\n";
        return EXIT_FAILURE;
    }

    std::string jpegPath = argv[1];

    JpegDataStream jpegStream(jpegPath);
    auto header = scanHeader(jpegStream);
    printHeader(*header);
    auto body = fillScans(jpegStream, header);
    header = nullptr; // Ownership moved into body
    dataToImage(body);

    return EXIT_SUCCESS;
}
