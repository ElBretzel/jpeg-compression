#include "huffman_code.hpp"
#include "jpeg_scan.hpp"
#include "jpeg_transform.hpp"

void prelude() {
    JpegDataStream jpegStream =
        JpegDataStream("/home/dluca/Documents/Epita/TIFO/jpeg-compression/demo/correct_format.jpg");
    auto header = scanHeader(jpegStream);
    auto body = fillScans(jpegStream, header);
    header = nullptr; // Ownership moved into body
    dataToImage(body);
}

int main() {
    prelude();
    return EXIT_SUCCESS;
}
