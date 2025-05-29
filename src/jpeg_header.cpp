#include "jpeg_header.hpp"

bool fillDQT(std::ifstream& jpegFile, std::array<Quantization, 4>& tables) {
    auto read_byte = [&jpegFile]() {
        return static_cast<uint8_t>(jpegFile.get());
    };

    uint8_t b1;
    uint8_t b2;
    uint16_t u1;
    uint16_t u2;

    u1 = (read_byte() << 8) | read_byte();
    u2 = u1 - 2;

    while (u2) {
        Quantization quant;
        b1 = read_byte();
        u2--;
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
            if (u2 == 0 && i < DQTVAL) {
                std::cerr << "Can not check validity of jpeg file: "
                          << "Quantization tables length invalid, terminated early" << std::endl;
                return false;
            }
            u2--;
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
    uint16_t u2;

    header->type = SOF0BAS; // Baseline

    u1 = (read_byte() << 8) | read_byte() - 2;
    if (u1-- < 1 || read_byte() != SOF0PRE) {
        std::cerr << "Can not check validity of jpeg file: " << "SOF0 detected yet wrong precision specified"
                  << std::endl;
        return false;
    }

    header->height = (read_byte() << 8) | read_byte();
    header->width = (read_byte() << 8) | read_byte();

    if (u1 < 2 || header->width == 0 || header->height == 0) {
        std::cerr << "Can not check validity of jpeg file: " << "File dim should be greater than 0" << std::endl;
        return false;
    }
    u1 -= 2;

    b1 = read_byte();
    if (u1-- < 1 || b1 != SOF0GRAY && b1 != SOF0RGB) {
        std::cerr << "Can not check validity of jpeg file: " << "Only grayscaled and RGB image supported" << std::endl;
        return false;
    }
    header->numberComponents = b1;

    if (u1 < b1 * SOF0LEN) {
        std::cerr << "Can not check validity of jpeg file: " << "Incorrect SOF0 size" << std::endl;
        return false;
    }

    for (uint8_t ci = 0; ci < b1; ci++) {
        uint8_t id = read_byte();
        if (id < SOF0GRAY || id > SOF0RGB) {
            std::cerr << "Can not check validity of jpeg file: " << "Channel ID weirdly specified" << std::endl;
            return false;
        }
        Channel channel = std::move(header->channels[id - 1]);
        if (channel.completed) {
            std::cerr << "Can not check validity of jpeg file: " << "Channel already been filled" << std::endl;
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
    }

    return true;
}

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

    if (b1 = read_byte(), b2 = read_byte(); b1 != MARKERSTART || (b2 < APP0 || b2 > APPF)) {
        std::cerr << "Can not check validity of jpeg file: " << "APP incorrect" << std::endl;
        return header;
    }

    if (!readLength(jpegFile, (read_byte() << 8) | read_byte() - 2)) {
        return header;
    }

    do {
        if (read_byte() != MARKERSTART) {
            std::cerr << "Can not check validity of jpeg file: " << "Marker not present" << std::endl;
            return header;
        }

        b2 = jpegFile.get();

        if (b2 == DQT) {
            if (!fillDQT(jpegFile, header->tables)) {
                return header;
            }
        } else if (b2 == SOF0) {
            if (!fillFrame(jpegFile, header)) {
                return header;
            }

        } else if (b2 == DHT) {
            std::cout << "DHT found" << std::endl;
        } else if (b2 == SOS) {

            std::cout << "SOS found" << std::endl;
        } else if (b2 == DNL) {

        } else if (b2 == DRI) {

        } else if (b2 == COM) {
            if (!readLength(jpegFile, (read_byte() << 8) | read_byte() - 2)) {
                return header;
            }
        } else if (b2 == EOI) {

        } else {
            std::cerr << "Marker not implemented: ";
            BYTE_TO_HEX(b2);
            std::cout << std::endl;
            return header;
        }

    } while (!jpegFile.eof());

    header->isValid = true;
    return header;
}

void printHeader(const Header& header) {
    std::cout << "Header valid: " << header.isValid << std::endl;
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

    std::cout << "======= SOF TABLE ========" << std::endl;
    std::cout << "Image width: " << header.width << std::endl;
    std::cout << "Image height: " << header.height << std::endl;
    std::cout << "Image type: 0x";
    BYTE_TO_HEX(header.type);
    std::cout << std::endl;
    std::cout << "Number of channels: " << static_cast<int>(header.numberComponents) << std::endl;
    for (auto i = 0; i < header.numberComponents; i++) {
        std::cout << "Channel ID: " << i << std::endl;
        std::cout << "Completed: " << header.channels[i].completed << std::endl;
        std::cout << "Horizontal sampling: " << static_cast<int>(header.channels[i].horizontalSampling) << std::endl;
        std::cout << "Vertical sampling: " << static_cast<int>(header.channels[i].verticalSampling) << std::endl;
        std::cout << "Quantization table ID: " << static_cast<int>(header.channels[i].quantizationId) << std::endl;
    }
}