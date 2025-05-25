#include "jpeg_header.hpp"

bool fillDQT(std::ifstream& jpegFile, std::array<Quantization, 4>& tables) {
    uint8_t b1;
    uint8_t b2;
    uint16_t u1;
    uint16_t u2;

    b1 = jpegFile.get();
    b2 = jpegFile.get();
    u1 = (b1 << 8) | b2;
    u2 = u1 - 2;

    while (u2) {
        Quantization quant;
        b1 = jpegFile.get();
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

        if (b2 == 0) {
            uint8_t i = 0;
            while (i < DQTVAL) {
                b1 = jpegFile.get();
                if (b1 == EOF) {
                    std::cerr << "Can not check validity of jpeg file: " << "DQT ended prematuraly" << std::endl;
                    return false;
                }
                quant.values[zigZagMap[i]] = b1;
                i++;
                if (u2 == 0 && i < DQTVAL) {
                    std::cerr << "Can not check validity of jpeg file: "
                              << "Quantization tables length invalid, terminated early" << std::endl;
                    return false;
                }
                u2--;
            }

        } else {
            uint8_t i = 0;
            while (i < DQTVAL) {
                b1 = jpegFile.get();
                b2 = jpegFile.get();
                if (b1 == EOF || b2 == EOF) {
                    std::cerr << "Can not check validity of jpeg file: " << "DQT ended prematuraly" << std::endl;
                    return false;
                }
                u1 = (b1 << 8) | b2;
                quant.values[zigZagMap[i]] = u1;
                i++;
                if (u2 < 2 && i < DQTVAL) {
                    std::cerr << "Can not check validity of jpeg file: "
                              << "Quantization tables length invalid, terminated early" << std::endl;
                    return false;
                }
                u2 -= 2;
            }
        }
        quant.completed = true;
        tables[quant.tableId] = quant;
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
    // https://en.wikipedia.org/wiki/JPEG_File_Interchange_Format
    // https://www.geocities.ws/crestwoodsdd/JPEG.htm
    // https://help.accusoft.com/ImageGear-Net/v24.12/Windows/HTML/JPEG_Non-Image_Data_Structure.html

    // SOI check
    b1 = jpegFile.get();
    b2 = jpegFile.get();
    u1 = (b1 << 8) | b2;
    if (u1 != SOI) {
        std::cerr << "Can not check validity of jpeg file: " << "SOI incorrect" << std::endl;
        return header;
    }

    // APPN check
    b1 = jpegFile.get();
    b2 = jpegFile.get();

    if (b1 != APPN || (b2 < APP0 || b2 > APPF)) {
        std::cerr << "Can not check validity of jpeg file: " << "APP incorrect" << std::endl;
        return header;
    }

    // APPN skip
    b1 = jpegFile.get();
    b2 = jpegFile.get();
    u1 = (b1 << 8) | b2;
    u2 = u1 - 2;

    while (u2--) {
        if (jpegFile.get() == EOF) {
            std::cerr << "Can not check validity of jpeg file: " << "APP ended prematuraly" << std::endl;
            return header;
        }
    }

    // DQT or HUFF
    b1 = jpegFile.get();
    b2 = jpegFile.get();
    u1 = (b1 << 8) | b2;

    if (u1 == DQT) {
        fillDQT(jpegFile, header->tables);
    } else {
        std::cerr << "Can not check validity of jpeg file: " << "Quantization tables not present" << std::endl;
        return header;
    }

    header->isValid = true;
    return header;
}

void printHeader(const Header& header) {
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