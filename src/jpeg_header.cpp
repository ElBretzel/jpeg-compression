#include "jpeg_header.hpp"

bool readLength(std::ifstream& jpegFile, uint16_t length) {
    while (length--) {
        uint8_t b = jpegFile.get();
        if (b == EOF) {
            std::cerr << "Can not check validity of jpeg file: " << "Read ended prematuraly" << std::endl;
            return false;
        }
    }
    return true;
}

bool fillDQT(std::ifstream& jpegFile, std::array<Quantization, 4>& tables) {
    auto read_byte = [&jpegFile]() {
        return static_cast<uint8_t>(jpegFile.get());
    };

    uint8_t b1;
    uint8_t b2;
    uint16_t u1;

    u1 = (read_byte() << 8) | read_byte() - 2;

    while (u1) {
        Quantization quant;
        b1 = read_byte();
        u1--;
        b2 = (b1 >> 4); // table precision
        b1 = b1 & 0x0F; // table id

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
        while (i < DQTVAL) {
            b1 = read_byte();
            if (b1 == EOF) {
                std::cerr << "Can not check validity of jpeg file: " << "DQT ended prematuraly" << std::endl;
                return false;
            }
            quant.values[reverseZigZagMap[i]] = b1;
            i++;
            if (u1 == 0 && i < DQTVAL) {
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

bool fillFrame(std::ifstream& jpegFile, std::unique_ptr<Header>& header) {
    auto read_byte = [&jpegFile]() {
        return static_cast<uint8_t>(jpegFile.get());
    };

    if (header->type != 0xFF) {
        std::cerr << "Can not check validity of jpeg file: " << "Image contains more than 1 frame" << std::endl;
        return false;
    }

    uint8_t b1;
    uint8_t b2;
    uint16_t u1;

    header->type = SOF0BAS; // Baseline

    u1 = (read_byte() << 8) | read_byte() - 2;
    if (!u1) {
        std::cerr << "Can not check validity of jpeg file: " << "Incorrect SOF0 size" << std::endl;
        return false;
    }

    if (read_byte() != SOF0PRE) {
        std::cerr << "Can not check validity of jpeg file: " << "SOF0 detected yet wrong precision specified"
                  << std::endl;
        return false;
    }

    if (--u1 < 4) {
        std::cerr << "Can not check validity of jpeg file: " << "Incorrect SOF0 size" << std::endl;
        return false;
    }

    header->height = (read_byte() << 8) | read_byte();
    header->width = (read_byte() << 8) | read_byte();

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

    b1 = read_byte();
    // Nf can be 0-255 but because this value is used in sof to identify comp id, we will restrict to 4 channels without
    // repetition
    if (!b1 || b1 > SOF0MAXCOMP) {
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
        uint8_t id = read_byte();

        if (header->appType == APPE) {
            id += 1;
        }
        // Cid can be 0, but we will suppose that ID is always entered between 1 and 4
        if (!id || id > SOF0MAXCOMP) {
            std::cerr << "Can not check validity of jpeg file: " << "Channel ID weirdly specified" << std::endl;
            return false;
        }
        Channel channel = std::move(header->channels[id - 1]);
        if (channel.completed) {
            std::cerr << "Can not check validity of jpeg file: " << "SOF channel already been filled" << std::endl;
            return false;
        }
        b2 = read_byte();
        channel.horizontalSampling = b2 >> 4;
        channel.verticalSampling = b2 & 0x0F;

        if (channel.horizontalSampling < SOF0MINSAMP || channel.horizontalSampling > SOF0MAXSAMP) {
            std::cerr << "Can not check validity of jpeg file: "
                      << "Horizontal sampling factor has incorrect number of data units" << std::endl;
            return false;
        }
        if (channel.verticalSampling < SOF0MINSAMP || channel.verticalSampling > SOF0MAXSAMP) {
            std::cerr << "Can not check validity of jpeg file: "
                      << "Vertical sampling factor has incorrect number of data units" << std::endl;
            return false;
        }
        channel.quantizationId = read_byte();
        if (channel.quantizationId > DQTMI) {
            std::cerr << "Can not check validity of jpeg file: " << "Quantization table ID overflow" << std::endl;
            return false;
        }
        channel.completed = true;
        header->channels[id - 1] = std::move(channel);
        u1 -= 3;
    }

    return true;
}

bool fillRestart(std::ifstream& jpegFile, std::unique_ptr<Header>& header) {
    auto read_byte = [&jpegFile]() {
        return static_cast<uint8_t>(jpegFile.get());
    };

    uint8_t b1;
    uint8_t b2;
    uint16_t u1;
    uint16_t u2;

    u1 = (read_byte() << 8) | read_byte();
    if (u1 != DRILEN) {
        std::cerr << "Can not check validity of jpeg file: " << "DRI marker has wrong size" << std::endl;
        return false;
    }

    header->restartInterval = (read_byte() << 8) | read_byte();
    return true;
}
bool fillApp(std::ifstream& jpegFile, std::unique_ptr<Header>& header) {

    auto read_byte = [&jpegFile]() {
        return static_cast<uint8_t>(jpegFile.get());
    };

    if (!readLength(jpegFile, (read_byte() << 8) | read_byte() - 2)) {
        return false;
    }
    return true;
}

bool fillDHT(std::ifstream& jpegFile, std::unique_ptr<Header>& header) {
    auto read_byte = [&jpegFile]() {
        return static_cast<uint8_t>(jpegFile.get());
    };

    uint8_t b1;
    uint8_t b2;
    uint16_t u1;
    uint16_t u2;

    u1 = (read_byte() << 8) | read_byte() - 2;

    while (u1) {
        b1 = read_byte();
        b2 = b1 >> 4;
        b1 = b1 & 0x0F;
        u1--;

        if (b1 > DHTMHT) {
            std::cerr << "Can not check validity of jpeg file: " << "Wrong Huffman table class" << std::endl;
            return false;
        }

        if (b2 > DHTMHT) {
            std::cerr << "Can not check validity of jpeg file: " << "Huffman table ID overflow" << std::endl;
            return false;
        }

        HuffmanTable huffTable = std::move(header->huffmanTable[b1 * 2 + b2]);

        if (huffTable.completed) {
            std::cerr << "Can not check validity of jpeg file: " << "Huffman table values already been treated"
                      << std::endl;
            return false;
        }

        huffTable.tableClass = b1;
        huffTable.identifier = b2;

        uint8_t totalSymbol = 0;

        for (uint8_t i = 0; i < DHTBITS; i++) {
            HuffmanCode code;
            code.codeLength = i + 1;
            if (u1-- == 0) {
                std::cerr << "Can not check validity of jpeg file: " << "DHT ended prematuraly" << std::endl;
                return false;
            }
            code.symbolCount = read_byte();
            code.huffVal.reserve(code.symbolCount);
            totalSymbol += code.symbolCount;
            huffTable.huffCode[i] = std::move(code);
        }

        if (u1 < totalSymbol) {
            std::cerr << "Can not check validity of jpeg file: " << "DHT size has not enough space to store all symbols"
                      << std::endl;
            return false;
        }

        for (uint8_t i = 0; i < DHTBITS; i++) {
            uint8_t symbolCount = huffTable.huffCode[i].symbolCount;
            for (uint8_t j = 0; j < symbolCount; j++) {
                huffTable.huffCode[i].huffVal.push_back(read_byte());
                u1--;
            }
        }

        huffTable.completed = true;
        header->huffmanTable[b1 * 2 + b2] = std::move(huffTable);
    }

    if (u1) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "DHT data left" << std::endl;
        return false;
    }

    return true;
}

bool fillSOS(std::ifstream& jpegFile, std::unique_ptr<Header>& header) {
    auto read_byte = [&jpegFile]() {
        return static_cast<uint8_t>(jpegFile.get());
    };

    uint8_t b1;
    uint8_t b2;
    uint16_t u1;
    uint16_t u2;

    u1 = read_byte() << 8 | read_byte() - 2;
    if (!u1) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "SOS wrong size" << std::endl;
        return false;
    }

    for (auto& c : header->channels) {
        c.completed = false;
    }

    b1 = read_byte();
    if (--u1 < 2 * b1) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "SOS can not store channels coding" << std::endl;
        return false;
    }

    // For simplicity, we will ignore the rule of following same order of components ID with the SOF
    // Sampling is also ignored for now (and maybe will never be implemented)

    for (uint8_t i = 0; i < b1; i++) {
        uint8_t id = read_byte();
        if (header->appType == APPE) {
            id += 1;
        }
        if (id > header->numberComponents) {
            std::cerr << "Can not check validity of jpeg file: "
                      << "SOS component ID is greater than the total number of components" << std::endl;
            return false;
        }
        Channel channel = std::move(header->channels[id - 1]);
        if (channel.completed) {
            std::cerr << "Can not check validity of jpeg file: " << "SOS channel already been filled" << std::endl;
            return false;
        }

        b2 = read_byte();
        channel.huffDCId = b2 >> 4;
        channel.huffACId = b2 & 0x0F;

        if (channel.huffACId > DHTMHT || channel.huffDCId > DHTMHT) {
            std::cerr << "Can not check validity of jpeg file: " << "SOS channel Huffman ID overflow" << std::endl;
            return false;
        }

        channel.completed = true;
        header->channels[id - 1] = std::move(channel);
        u1 -= 2;
    }
    if (u1 != 3) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "SOS wrong size" << std::endl;
        return false;
    }
    if (b1 = read_byte(), b2 = read_byte(); b1 != SOSSPECS || b2 != SOSSPECE) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "Non baseline spectral selection is not supported" << std::endl;
        return false;
    }
    b1 = read_byte();
    b2 = b1 >> 4;
    b1 = b1 & 0x0F;
    if (b1 != SOSSUCC || b2 != SOSSUCC) {
        std::cerr << "Can not check validity of jpeg file: "
                  << "Non baseline successive approximation position bit is not supported" << std::endl;
        return false;
    }

    return true;
}

std::unique_ptr<Header> scanHeader(const std::string& filePath) {
    // Some utility variables
    uint8_t b1;
    uint8_t b2;
    uint16_t u1;
    uint16_t u2;

    auto header = std::make_unique<Header>();
    header->isValid = false;

    if (filePath.empty()) {
        std::cerr << "Can not check validity of jpeg file: " << "path is empty" << std::endl;
        return header;
    }
    if (!filePath.ends_with(".jpg") && !filePath.ends_with(".jpeg")) {
        std::cerr << "Can not check validity of jpeg file: " << "file has incorrect format" << std::endl;
        return header;
    }
    std::ifstream jpegFile(filePath, std::ios::binary);
    if (!jpegFile.is_open()) {
        std::cerr << "Can not check validity of jpeg file: " << "file can't be opened" << std::endl;
        return header;
    }
    auto read_byte = [&jpegFile]() {
        return static_cast<uint8_t>(jpegFile.get());
    };

    // https://www.w3.org/Graphics/JPEG/itu-t81.pdf
    if (read_byte() != MARKERSTART || read_byte() != SOI) {
        std::cerr << "Can not check validity of jpeg file: " << "SOI incorrect" << std::endl;
        return header;
    }

    do {
        if (read_byte() != MARKERSTART) {
            std::cerr << "Can not check validity of jpeg file: " << "Marker not present" << std::endl;
            return header;
        }

        b2 = jpegFile.get();

        if (b2 >= APP0 && b2 <= APPF) {
            header->appType = b2;
            if (!fillApp(jpegFile, header)) {
                return header;
            }
        }

        else if (b2 == DQT) {
            if (!fillDQT(jpegFile, header->tables)) {
                return header;
            }
        } else if (b2 == SOF0) {
            if (!fillFrame(jpegFile, header)) {
                return header;
            }

        } else if (b2 == DHT) {
            if (!fillDHT(jpegFile, header)) {
                return header;
            }
        } else if (b2 == SOS) {
            if (!fillSOS(jpegFile, header)) {
                return header;
            }
            break;
        } else if (b2 == DRI) {
            if (!fillRestart(jpegFile, header)) {
                return header;
            }

        } else if (b2 == EOI) {
            std::cerr << "Can not check validity of jpeg file: " << "EOI can not be found before SOS" << std::endl;
            return header;
        } else if (b2 == MARKERSTART) {
            // Fill byte, skip...
        } else if (!b2) {
            std::cerr << "Can not check validity of jpeg file: " << "Marker can not be 0" << std::endl;
            return header;
        } else {
            std::cerr << "Can not check validity of jpeg file: " << "Marker not implemented" << std::endl;
            BYTE_TO_HEX(b2);
            std::cout << std::endl;
            return header;
        }

    } while (!jpegFile.eof());

    header->isValid = true;
    return header;
}

void printDQTTable(const Header& header) {
    std::cout << "======= DQT TABLE ========" << std::endl;
    for (uint8_t i = 0; i < DQTMI + 1; i++) {
        if (!header.tables[i].completed) {
            continue;
        }
        std::cout << "| TABLE ID: " << static_cast<int>(i);
        for (uint8_t j = 0; j < DQTVAL; j++) {
            if (j % DQTBLOC == 0) {
                std::cout << std::endl;
            }
            BYTE_TO_HEX(static_cast<uint8_t>(header.tables[i].values[j]));
        }
        std::cout << std::endl;
    }
}

void printSOFTable(const Header& header) {
    std::cout << "======= SOF TABLE ========" << std::endl;
    std::cout << "Image convention type: ";
    BYTE_TO_HEX(header.appType);
    std::cout << std::endl;
    std::cout << "Image width: " << header.width << std::endl;
    std::cout << "Image height: " << header.height << std::endl;
    std::cout << "Image type: 0x";
    BYTE_TO_HEX(header.type);
    std::cout << std::endl;
    std::cout << "Number of channels: " << static_cast<int>(header.numberComponents) << std::endl;

    for (int i = 0; i < header.numberComponents; ++i) {
        const auto& ch = header.channels[i];
        std::cout << "Channel ID: " << i << std::endl;
        std::cout << "  Completed: " << ch.completed << std::endl;
        std::cout << "  Horizontal sampling: " << static_cast<int>(ch.horizontalSampling) << std::endl;
        std::cout << "  Vertical sampling: " << static_cast<int>(ch.verticalSampling) << std::endl;
        std::cout << "  Quantization table ID: " << static_cast<int>(ch.quantizationId) << std::endl;
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

        std::cout << "HUFFSIZE: { ";
        for (const auto& code : table.huffCode) {
            std::cout << static_cast<int>(code.symbolCount) << ", ";
        }
        std::cout << "}" << std::endl;
        std::cout << "HUFFVAL: {";
        for (const auto& code : table.huffCode) {
            std::cout << "[";
            for (const auto val : code.huffVal) {
                BYTE_TO_HEX(val);
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