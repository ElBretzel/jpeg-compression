#include "jpeg_body.hpp"

std::unique_ptr<Body> scanBody(std::ifstream& jpegFile, std::unique_ptr<Header>& header) {
    auto body = std::make_unique<Body>();
    body->header = std::move(header);

    if (!body->header->isValid) {
        std::cerr << "Body error: Won't read body until header is valid..." << std::endl;
        return body;
    }

    auto read_byte = [&jpegFile]() {
        return static_cast<uint8_t>(jpegFile.get());
    };

    uint8_t b1;
    uint8_t b2;

    b1 = read_byte();
    bool EOIRead = false;
    while (!jpegFile.eof()) {

        b2 = b1;
        b1 = read_byte();

        if (b2 == MARKERSTART) {
            if (b1 == EOI) {
                EOIRead = true;
                break;
            } else if (b1 >= RST0 && b1 <= RST7) {
                b1 = read_byte();
            } else if (b1 == MARKERSTART) {
            } else if (!b1) {
                body->data.addByte(b2);
                b1 = read_byte();
            } else {
                std::cerr << "Body error: " << "data invalid marker: ";
                BYTE_TO_HEX(b1);
                std::cerr << std::endl;
                return body;
            }
        } else {
            body->data.addByte(b2);
        }
    }

    if (!EOIRead) {
        return body;
    }

    body->isValid = true;
    return body;
}