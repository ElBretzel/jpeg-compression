#include "jpeg_scan.hpp"

bool scanProgressiveMarker(JpegDataStream& jpegStream, std::unique_ptr<Body>& body, uint8_t type) {
    switch (type) {
    case DRI:
        if (!fillRestart(jpegStream, body->header)) {
            std::cerr << "Body error: " << "could not read subsequent restart interval" << std::endl;
            return false;
        }
        printDRITable(*body->header);
        return true;
    case DQT:
        if (!fillDQT(jpegStream, body->header->quantTable)) {
            std::cerr << "Body error: " << "could not read subsequent DQT" << std::endl;
            return false;
        }
        printDQTTable(*body->header);
        return true;
    case DHT:
        if (!fillDHT(jpegStream, body->header)) {
            std::cerr << "Body error: " << "could not read subsequent DHT" << std::endl;
            return false;
        }
        printDHTTable(*body->header);
        return true;
    case SOS:
        // dataToImage(body);
        if (!fillSOS(jpegStream, body->header)) {
            std::cerr << "Could not read subsequent SOS marker" << std::endl;
            return false;
        }
        if (!decodeHuffman(body)) {
            std::cerr << "Could not decode subsequent Huffman data" << std::endl;
            return false;
        }
        printSOSTable(*body->header);
        break;
    default:
        std::cerr << "Body error SOF2: " << "data invalid marker: ";
        BYTE_TO_HEX(type);
        std::cerr << std::endl;
        return false;
    }
    return true;
}

bool scanMarker(JpegDataStream& jpegStream, std::unique_ptr<Body>& body, uint8_t type) {
    switch (type) {
    case RST0:
    case RST1:
    case RST2:
    case RST3:
    case RST4:
    case RST5:
    case RST6:
    case RST7:
        return true;
    case APP0:
    case APP1:
    case APP2:
    case APP3:
    case APP4:
    case APP5:
    case APP6:
    case APP7:
    case APP8:
    case APP9:
    case APPA:
    case APPB:
    case APPC:
    case APPD:
    case APPE:
    case APPF:
        return fillApp(jpegStream, body->header);
    case COM:
        return fillCom(jpegStream, body->header);
    case MARKERSTART:
        return true;
    default:
        if (body->header->type == SOF2) {
            return scanProgressiveMarker(jpegStream, body, type);
        } else {
            std::cerr << "Body error: " << "data invalid marker: ";
            BYTE_TO_HEX(type);
            std::cerr << std::endl;
            return false;
        }
    }
}

std::unique_ptr<Body> fillScans(JpegDataStream& jpegStream, std::unique_ptr<Header>& header) {
    auto body = std::make_unique<Body>(jpegStream);
    body->header = std::move(header);

    if (!body->header->isValid) {
        std::cerr << "Body error: Won't read body until header is valid..." << std::endl;
        return body;
    }

    auto mcu = std::make_unique<MCU>();
    auto mcuDecoded = std::make_unique<MCU>();

    mcu->mcuWidth = (body->header->width + 7) / 8;
    mcu->mcuHeight = (body->header->height + 7) / 8;
    mcu->mcuData.resize(mcu->mcuWidth * mcu->mcuHeight);

    mcuDecoded->mcuWidth = mcu->mcuWidth;
    mcuDecoded->mcuHeight = mcu->mcuHeight;
    mcuDecoded->mcuData.reserve(mcuDecoded->mcuWidth * mcuDecoded->mcuHeight);

    for (auto& mcuData : mcu->mcuData) {
        for (auto i = 0; i < body->header->numberComponents; i++) {
            mcuData[i].fill(0);
        }
    }
    body->mcu = std::move(mcu);
    body->mcuDecoded = std::move(mcuDecoded);

    // First scan
    if (!fillSOS(jpegStream, body->header)) {
        std::cerr << "FillScans: could not read first scan..." << std::endl;
        return body;
    }

    // First AC DC

    decodeHuffman(body);

    while (!jpegStream.isEOF()) {
        std::cout << jpegStream.tell() << " - " << std::to_string(jpegStream.bitPos) << " - "
                  << std::to_string(jpegStream.currentByte) << std::endl;
        uint8_t prefix = jpegStream.readByte();
        uint8_t type = jpegStream.readByte();
        BYTE_TO_HEX(prefix);
        BYTE_TO_HEX(type);
        std::cout << std::endl;

        if (prefix != MARKERSTART) {
            std::cerr << "Body error: " << "not valid marker prefix: ";
            BYTE_TO_HEX(prefix);
            std::cerr << std::endl;
            return body;
        }
        if (type == EOI) {
            break;
        } else if (!scanMarker(jpegStream, body, type)) {
            return body;
        }
    }

    body->isValid = true;
    return body;
}
