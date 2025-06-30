#include "jpeg_scan.hpp"

std::unique_ptr<Body> fillScans(JpegDataStream& jpegStream, std::unique_ptr<Header>& header) {
    auto body = std::make_unique<Body>(jpegStream);
    body->header = std::move(header);

    if (!body->header->isValid) {
        std::cerr << "Body error: Won't read body until header is valid..." << std::endl;
        return body;
    }

    auto mcu = std::make_unique<MCU>();
    mcu->mcuWidth = (body->header->width + 7) / 8;
    mcu->mcuHeight = (body->header->height + 7) / 8;
    mcu->mcuData.resize(mcu->mcuWidth * mcu->mcuHeight);
    for (auto& mcuData : mcu->mcuData) {
        for (auto i = 0; i < body->header->numberComponents; i++) {
            mcuData[i].fill(0);
        }
    }
    body->mcu = std::move(mcu);

    // First scan
    if (!fillSOS(jpegStream, body->header)) {
        std::cerr << "FillScans: could not read first scan..." << std::endl;
        return body;
    }

    // First AC DC
    decodeHuffman(body);

    uint8_t b1;
    uint8_t b2;

    // while (!jpegStream.isEOF()) {
    // }

    // b1 = jpegStream.readByte();
    // bool EOIRead = false;
    // while (!jpegStream.isEOF()) {

    //     b2 = b1;
    //     b1 = jpegStream.readByte();

    //     if (b2 == MARKERSTART) {
    //         if (b1 == EOI) {
    //             EOIRead = true;
    //             break;
    //         } else if (b1 >= RST0 && b1 <= RST7) {
    //             b1 = jpegStream.readByte();
    //         } else if (b1 == MARKERSTART) {
    //         } else if (!b1) {
    //             body->data.addByte(b2);
    //             b1 = read_byte();
    //         } else {
    //             std::cerr << "Body error: " << "data invalid marker: ";
    //             BYTE_TO_HEX(b1);
    //             std::cerr << std::endl;
    //             return body;
    //         }
    //     } else {
    //         body->data.addByte(b2);
    //     }
    // }

    // if (!EOIRead) {
    //     return body;
    // }

    body->isValid = true;
    return body;
}

// std::unique_ptr<Header> writeHeader(std::ifstream& ppmFile, std::unique_ptr<Body>& body) {
//     // Some utility variables
//     uint8_t b1;
//     uint8_t b2;
//     uint16_t u1;

//     auto header = std::make_unique<Header>();
//     header->isValid = false;

//     if (!ppmFile.is_open()) {
//         std::cerr << "Can not check validity of jpeg file: " << "file can't be opened" << std::endl;
//         return header;
//     }
//     auto read_byte = [&ppmFile]() {
//         return static_cast<uint8_t>(ppmFile.get());
//     };
//     body->data.addByte(MARKERSTART);
//     body->data.addByte(SOI);

//     header->appType = APP0;
//     body->data.addByte(MARKERSTART);
//     body->data.addByte(header->appType);
//     body->data.addByte(0x00);

//     body->header->restartInterval = 0;
//     body->data.addByte(MARKERSTART);
//     body->data.addByte(DRI);
//     body->data.addByte(DRILEN);
//     body->data.addByte(static_cast<uint8_t>(body->header->restartInterval >> 8));
//     body->data.addByte(static_cast<uint8_t>(body->header->restartInterval & 0xFF));

//     return header;
// }
