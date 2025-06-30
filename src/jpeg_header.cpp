#include "jpeg_header.hpp"

bool fillDQT(JpegDataStream& jpegStream, std::array<Quantization, 4>& tables) {
    uint8_t b1;
    uint8_t b2;
    uint16_t u1;

    u1 = (jpegStream.readByte() << 8) | jpegStream.readByte() - 2;

    while (u1) {
        Quantization quant;
        b2 = jpegStream.readBits(4); // table precision
        b1 = jpegStream.readBits(4); // table id
        u1--;

        if (b1 > DQTMI) {
            std::cerr << "Can not check validity of jpeg file: " << "Quantization tables ID is invalid" << std::endl;
            return false;
        }
        if (b2 > DQTMP) {
            std::cerr << "Can not check validity of jpeg file: " << "Quantization tables precision is invalid"
                      << std::endl;
            return false;
        }

        quant.tableId = b1;
        quant.tablePrecision = b2;

        if (b2 != 0) {
            std::cerr << "Can not check validity of jpeg file: "
                      << "Sequential baseline only supported. Precision overflow" << std::endl;
            return false;
        }

        uint8_t i = 0;
        while (i < MCUX) {
            b1 = jpegStream.readByte();
            if (b1 == EOF) {
                std::cerr << "Can not check validity of jpeg file: " << "DQT ended prematuraly" << std::endl;
                return false;
            }
            quant.values[reverseZigZagMap[i]] = b1;
            i++;
            if (u1 == 0 && i < MCUX) {
                std::cerr << "Can not check validity of jpeg file: "
                          << "Quantization tables length invalid, terminated early" << std::endl;
                return false;
            }
            u1--;
        }

        quant.completed = true;
        tables[quant.tableId] = quant;
    }

    return true;
}

bool fillFrame(JpegDataStream& jpegStream, std::unique_ptr<Header>& header, const uint8_t type) {
    if (header->type != 0xFF) {
        std::cerr << "Can not check validity of jpeg file: " << "Image contains more than 1 frame" << std::endl;
        return false;
    }

    uint8_t b1;
    uint8_t b2;
    uint16_t u1;

    header->type = type;

    u1 = (jpegStream.readByte() << 8) | jpegStream.readByte() - 2;
    if (!u1) {
        std::cerr << "Can not check validity of jpeg file: " << "Incorrect SOF0 size" << std::endl;
        return false;
    }

    if (type == SOF0) {
        b1 = jpegStream.readByte();
        if (b1 != SOF0PRE) {
            std::cerr << "Can not check validity of jpeg file: " << "SOF0 detected yet wrong precision specified"
                      << std::endl;
            return false;
        }
        header->precision = b1;
    } else {
        b1 = jpegStream.readByte();
        if (b1 < SOF2MINPRE || b1 > SOF2MAXPRE) {
            std::cerr << "Can not check validity of jpeg file: " << "SOF2 detected yet wrong precision specified"
                      << std::endl;
            return false;
        }
        header->precision = b1;
    }

    if (--u1 < 4) {
        std::cerr << "Can not check validity of jpeg file: " << "Incorrect SOF0 size" << std::endl;
        return false;
    }

    header->height = (jpegStream.readByte() << 8) | jpegStream.readByte();
    header->width = (jpegStream.readByte() << 8) | jpegStream.readByte();

    if (header->width == 0 || header->height == 0) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "File dimension should be known, live encoding is not supported" << std::endl;
        return false;
    }
    u1 -= 4;

    if (!u1) {
        std::cerr << "Can not check validity of jpeg file: " << "Incorrect SOF0 size" << std::endl;
        return false;
    }

    b1 = jpegStream.readByte();
    // Nf can be 0-255 but because this value is used in sof to identify comp id, we will restrict to 4 channels without
    // repetition
    if (!b1 || b1 > SOFMAXCOMP) {
        std::cerr << "Can not check validity of jpeg file: " << "Image should have 1 to 4 channels" << std::endl;
        return false;
    }
    header->numberComponents = b1;
    u1--;

    if (u1 != b1 * 3) {
        std::cerr << "Can not check validity of jpeg file: " << "Incorrect SOF0 size" << std::endl;
        return false;
    }

    for (int8_t ci = 0; ci < b1; ci++) {
        uint8_t id = jpegStream.readByte();

        if (ci == 0) {
            header->component_offset = id;
        }
        id -= header->component_offset;

        if (id > SOFMAXCOMP - 1) {
            std::cerr << "Can not check validity of jpeg file: " << "Channel ID weirdly specified" << std::endl;
            return false;
        }
        Channel channel = std::move(header->channels[id]);
        if (channel.frame_completed) {
            std::cerr << "Can not check validity of jpeg file: " << "SOF channel already been filled" << std::endl;
            return false;
        }
        channel.horizontalSampling = jpegStream.readBits(4);
        channel.verticalSampling = jpegStream.readBits(4);

        if (channel.horizontalSampling < SOFMINSAMP || channel.horizontalSampling > SOFMAXSAMP) {
            std::cerr << "Can not check validity of jpeg file: "
                      << "Horizontal sampling factor has incorrect number of data units" << std::endl;
            return false;
        }
        if (channel.verticalSampling < SOFMINSAMP || channel.verticalSampling > SOFMAXSAMP) {
            std::cerr << "Can not check validity of jpeg file: "
                      << "Vertical sampling factor has incorrect number of data units" << std::endl;
            return false;
        }
        channel.quantizationId = jpegStream.readByte();
        if (channel.quantizationId > DQTMI) {
            std::cerr << "Can not check validity of jpeg file: " << "Quantization table ID overflow" << std::endl;
            return false;
        }
        channel.frame_completed = true;
        header->channels[id] = std::move(channel);
        u1 -= 3;
    }

    return true;
}

bool fillRestart(JpegDataStream& jpegStream, std::unique_ptr<Header>& header) {
    uint8_t b1;
    uint8_t b2;
    uint16_t u1;

    u1 = (jpegStream.readByte() << 8) | jpegStream.readByte();
    if (u1 != DRILEN) {
        std::cerr << "Can not check validity of jpeg file: " << "DRI marker has wrong size" << std::endl;
        return false;
    }

    header->restartInterval = (jpegStream.readByte() << 8) | jpegStream.readByte();
    return true;
}
bool fillApp(JpegDataStream& jpegStream, std::unique_ptr<Header>& header) {
    uint16_t length = (jpegStream.readByte() << 8) | jpegStream.readByte() - 2;

    for (uint16_t i = 0; i < length; i++) {
        if (jpegStream.readByte() == EOF) {
            std::cerr << "Unexpected EOF in APP segment\n";
            return false;
        }
    }
    return true;
}

bool fillDHT(JpegDataStream& jpegStream, std::unique_ptr<Header>& header) {
    uint8_t b1;
    uint8_t b2;
    uint16_t u1;

    u1 = (jpegStream.readByte() << 8) | jpegStream.readByte() - 2;

    while (u1) {
        b2 = jpegStream.readBits(4);
        b1 = jpegStream.readBits(4);
        u1--;

        if (b2 > DHTMHT) {
            std::cerr << "Can not check validity of jpeg file: " << "Wrong Huffman table class" << std::endl;
            return false;
        }

        if (header->type == SOF0 && b1 > DHTMHT || header->type == SOF2 && b1 > DHTMHT2) {
            std::cerr << "Can not check validity of jpeg file: " << "Huffman table ID overflow" << std::endl;
            return false;
        }

        HuffmanTable huffTable = std::move(header->huffmanTable[b2 * 2 + b1]);

        if (huffTable.completed) {
            std::cerr << "Can not check validity of jpeg file: " << "Huffman table values already been treated"
                      << std::endl;
            return false;
        }

        huffTable.tableClass = b2;
        huffTable.identifier = b1;

        uint8_t totalSymbol = 0;

        for (uint8_t i = 0; i < DHTBITS; i++) {
            HuffmanData code;
            code.codeLength = i + 1;
            if (u1-- == 0) {
                std::cerr << "Can not check validity of jpeg file: " << "DHT ended prematuraly" << std::endl;
                return false;
            }
            code.symbolCount = jpegStream.readByte();
            code.huffVal.reserve(code.symbolCount);
            code.huffCode.reserve(code.symbolCount);
            totalSymbol += code.symbolCount;
            huffTable.huffData[i] = std::move(code);
        }

        if (u1 < totalSymbol) {
            std::cerr << "Can not check validity of jpeg file: " << "DHT size has not enough space to store all symbols"
                      << std::endl;
            return false;
        }

        for (uint8_t i = 0; i < DHTBITS; i++) {
            uint8_t symbolCount = huffTable.huffData[i].symbolCount;
            // Maybe add additional check for Huffman entropy coding (bit overflow)
            for (uint8_t j = 0; j < symbolCount; j++) {
                huffTable.huffData[i].huffVal.push_back(jpegStream.readByte());
                u1--;
            }
        }

        huffTable.completed = true;
        header->huffmanTable[b2 * 2 + b1] = std::move(huffTable);
    }

    if (u1) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "DHT data left" << std::endl;
        return false;
    }

    return true;
}

bool fillSOS(JpegDataStream& jpegStream, std::unique_ptr<Header>& header) {
    uint8_t b1;
    uint8_t b2;
    uint16_t u1;

    u1 = jpegStream.readByte() << 8 | jpegStream.readByte() - 2;
    if (!u1) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "SOS wrong size" << std::endl;
        return false;
    }

    b1 = jpegStream.readByte();
    if (--u1 < 2 * b1) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "SOS can not store channels coding" << std::endl;
        return false;
    }

    // For simplicity, we will ignore the rule of following same order of components ID with the SOF
    // Sampling is also ignored for now (and maybe will never be implemented)

    for (uint8_t i = 0; i < b1; i++) {
        uint8_t id = jpegStream.readByte();

        id -= header->component_offset;
        if (id > header->numberComponents) {
            std::cerr << "Can not check validity of jpeg file: "
                      << "SOS component ID is greater than the total number of components" << std::endl;
            return false;
        }
        Channel channel = std::move(header->channels[id]);
        if (!channel.frame_completed) {
            std::cerr << "Channel frame was not completed before" << std::endl;
            return false;
        }
        if (channel.scan_completed) {
            std::cerr << "Can not check validity of jpeg file: " << "SOS channel scan already been filled" << std::endl;
            return false;
        }

        channel.huffDCId = jpegStream.readBits(4);
        channel.huffACId = jpegStream.readBits(4);

        if (channel.huffACId > DHTMHT || channel.huffDCId > DHTMHT) {
            std::cerr << "Can not check validity of jpeg file: " << "SOS channel Huffman ID overflow" << std::endl;
            return false;
        }

        channel.scan_completed = true;
        header->channels[id] = std::move(channel);
        u1 -= 2;
    }
    if (u1 != 3) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "SOS wrong size" << std::endl;
        return false;
    }

    b1 = jpegStream.readByte();
    if (header->type == SOF0 && b1) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "Baseline spectral selection is not supported" << std::endl;
        return false;
    }
    if (header->type == SOF2 && b1 > EOS) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "Spectral selection start is too high" << std::endl;
        return false;
    }
    header->progressiveInfo.startOfSpectral = b1;

    b1 = jpegStream.readByte();
    if (header->type == SOF0 && b1 != EOS) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "Baseline spectral selection is not supported" << std::endl;
        return false;
    }
    if (header->type == SOF2 && b1 < header->progressiveInfo.startOfSpectral) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "Spectral selection end is too low" << std::endl;
        return false;
    }
    header->progressiveInfo.endOfSpectral = b1;

    b2 = jpegStream.readBits(4);
    b1 = jpegStream.readBits(4);
    if (header->type == SOF0 && (b1 || b2)) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "Baseline successive approximation position bit is not supported" << std::endl;
        return false;
    }
    if (header->type == SOF2 && (b1 > SOSSUCCBITMAX || b2 > SOSSUCCBITMAX)) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "Successive approximation bit is too high" << std::endl;
        return false;
    }
    header->progressiveInfo.successiveBitHigh = b2;
    header->progressiveInfo.successiveBitLow = b1;

    return true;
}

std::unique_ptr<Header> scanHeader(JpegDataStream& jpegStream) {
    // Some utility variables
    uint8_t b1;
    uint8_t b2;
    uint16_t u1;

    auto header = std::make_unique<Header>();
    header->isValid = false;

    // https://www.w3.org/Graphics/JPEG/itu-t81.pdf
    if (jpegStream.readByte() != MARKERSTART || jpegStream.readByte() != SOI) {
        std::cerr << "Can not check validity of jpeg file: " << "SOI incorrect" << std::endl;
        return header;
    }

    bool done = false;

    do {
        auto i = jpegStream.readByte();
        if (i != MARKERSTART) {
            std::cerr << "Can not check validity of jpeg file: " << "Marker not present" << std::endl;
            return header;
        }

        b2 = jpegStream.readByte();

        switch (b2) {
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
            if (!fillApp(jpegStream, header)) {
                return header;
            }
            break;
        case DQT:
            if (!fillDQT(jpegStream, header->quantTable)) {
                return header;
            }
            break;

        case SOF0:
        case SOF2:
            if (!fillFrame(jpegStream, header, b2)) {
                return header;
            }
            break;

        case DHT:
            if (!fillDHT(jpegStream, header)) {
                return header;
            }
            break;

        case SOS:
            header->isValid = true;
            return header;
        case DRI:
            if (!fillRestart(jpegStream, header)) {
                return header;
            }
            break;
        case EOI:
            std::cerr << "Can not check validity of jpeg file: EOI can not be found before SOS" << std::endl;
            return header;

        case MARKERSTART:
            // Fill byte, skip...
            break;

        case 0:
            std::cerr << "Can not check validity of jpeg file: Marker can not be 0" << std::endl;
            return header;

        default:
            std::cerr << "Can not check validity of jpeg file: Marker not implemented" << std::endl;
            BYTE_TO_HEX(b2);
            std::cerr << std::endl;
            return header;
        }

    } while (true);
}

void printDQTTable(const Header& header) {
    std::cout << "======= DQT TABLE ========" << std::endl;
    for (uint8_t i = 0; i < DQTMI + 1; i++) {
        if (!header.quantTable[i].completed) {
            continue;
        }
        std::cout << "| TABLE ID: " << static_cast<int>(i);
        for (uint8_t j = 0; j < MCUX; j++) {
            if (j % DQTBLOC == 0) {
                std::cout << std::endl;
            }
            BYTE_TO_HEX(static_cast<uint8_t>(header.quantTable[i].values[j]));
        }
        std::cout << std::endl;
    }
}

void printSOFTable(const Header& header) {
    std::cout << "======= SOF TABLE ========" << std::endl;
    std::cout << "Precision: " << header.precision << std::endl;
    std::cout << "Image width: " << header.width << std::endl;
    std::cout << "Image height: " << header.height << std::endl;
    std::cout << "Component offset: " << header.component_offset << std::endl;
    std::cout << "Image type: 0x";
    BYTE_TO_HEX(header.type);
    std::cout << std::endl;
    std::cout << "Number of channels: " << static_cast<int>(header.numberComponents) << std::endl;

    for (int i = 0; i < header.numberComponents; ++i) {
        const auto& ch = header.channels[i];
        std::cout << "Channel ID: " << i << std::endl;
        std::cout << "  Frame completed: " << ch.frame_completed << std::endl;
        std::cout << "  Scan completed: " << ch.scan_completed << std::endl;
        std::cout << "  Horizontal sampling: " << static_cast<int>(ch.horizontalSampling) << std::endl;
        std::cout << "  Vertical sampling: " << static_cast<int>(ch.verticalSampling) << std::endl;
        std::cout << "  Quantization table ID: " << static_cast<int>(ch.quantizationId) << std::endl;
        std::cout << "  AC Huffman table ID: " << static_cast<int>(ch.huffACId) << std::endl;
        std::cout << "  DC Huffman table ID: " << static_cast<int>(ch.huffDCId) << std::endl;
    }
}

void printDRITable(const Header& header) {
    std::cout << "======= DRI TABLE ========" << std::endl;
    std::cout << "Restart interval: " << static_cast<int>(header.restartInterval) << std::endl;
}

void printDHTTable(const Header& header) {
    std::cout << "======= DHT TABLE ========" << std::endl;
    for (const auto& table : header.huffmanTable) {
        std::cout << "Table class: " << static_cast<int>(table.tableClass) << std::endl;
        std::cout << "Table class ID: " << static_cast<int>(table.identifier) << std::endl;
        std::cout << "Table completed: " << table.completed << std::endl;

        std::cout << "BITS: { ";
        for (const auto& code : table.huffData) {
            std::cout << static_cast<int>(code.symbolCount) << ", ";
        }
        std::cout << "}" << std::endl;
        std::cout << "HUFFVAL: {";
        for (const auto& code : table.huffData) {
            std::cout << "[";
            for (const auto val : code.huffVal) {
                BYTE_TO_HEX(val);
            }
            std::cout << "]; ";
        }
        std::cout << "}" << std::endl;
        std::cout << "HUFFSIZE: {";
        for (const auto& code : table.huffData) {
            if (!code.symbolCount) {
                continue;
            }
            std::cout << "[";
            for (auto j = 0; j < code.symbolCount; j++) {
                std::cout << static_cast<int>(code.codeLength) << " ";
            }
            std::cout << "]; ";
        }
        std::cout << "}" << std::endl;
    }
}

void printHeader(const Header& header) {
    std::cout << "Header valid: " << header.isValid << std::endl;
    printDQTTable(header);
    printSOFTable(header);
    printDRITable(header);
    printDHTTable(header);
}