#include "decode_huffman.hpp"

bool generateCode(std::unique_ptr<Body>& body) {

    for (auto& table : body->header->huffmanTable) {
        uint16_t code = 0;
        for (auto& data : table.huffData) {
            if (!data.symbolCount) {
                code <<= 1;
                continue;
            }

            if (data.symbolCount > (1 << data.codeLength)) {
                std::cerr << "Can not generate code: "
                          << "There is too many symbols to generate code of fixed length" << std::endl;
                return false;
            }
            for (uint8_t i = 0; i < data.symbolCount; i++) {
                data.huffCode.push_back(code);
                code++;
            }
            code <<= 1;
        }
    }

    return true;
}

// Order Huffman codes and symbols for one Huffman table
bool fillDecodeTable(const HuffmanTable& table, HuffmanDecodeTable& decodeTable) {
    for (auto& data : table.huffData) {
        if (data.huffCode.size() != data.huffVal.size()) {
            std::cerr << "Could not generate decode table for table " << table.tableClass << " " << table.identifier
                      << " : Size mitchmatch" << std::endl;
            return false;
        }

        for (uint8_t i = 0; i < data.symbolCount; i++) {
            uint16_t code = data.huffCode[i];
            uint8_t symbol = data.huffVal[i];
            decodeTable[data.codeLength - 1][code] = symbol;
        }
    }
    return true;
}

uint8_t decodeSymbol(JpegDataStream& stream, const HuffmanDecodeTable& decodeTable) {

    uint16_t code = 0;
    // codeLength is shifted by 1
    for (uint8_t codeLength = 0; codeLength < DHTBITS; codeLength++) {
        uint8_t b = stream.readBit();
        if (b == 0xFF) {
            return 0xFF;
        }
        code = (code << 1) | b;
        auto it = decodeTable[codeLength].find(code);
        if (it != decodeTable[codeLength].end()) {
            return it->second;
        }
    }

    std::cerr << "Decode symbol error: Code not found in the decode table..." << std::endl;
    return 0xFF;
}

bool getACDCSymbol(JpegDataStream& stream, int16_t& symbol, const uint8_t length) {

    uint16_t s = stream.readBits(length);
    if (s == 0xFFFF) {
        return false;
    }
    // Signed decoding >>>> Leftmost half is negative, Rightmost half is positive
    // Leftmost half
    if (s < (1 << (length - 1))) {
        // (1 << dcLength) - 1 == 2^length starting from 0
        // so we shift in negative our s
        symbol = s - ((1 << length) - 1);
    }
    // Rightmost half
    else {
        symbol = s;
    }

    return true;
}

bool decodeMCU(JpegDataStream& stream, MCUComponents& mcu, const HuffmanDecodeTable& dc, const HuffmanDecodeTable& ac,
               int16_t& previousDC) {

    // DC decoding
    uint8_t dcLength = decodeSymbol(stream, dc);
    if (dcLength == 0xFF) {
        std::cerr << "Decode error: DC decoding sequence corrupted" << std::endl;
        return false;
    }

    int16_t dcSymbol = 0;
    if (dcLength > 0) {
        if (!getACDCSymbol(stream, dcSymbol, dcLength)) {
            return false;
        }
    }

    mcu[reverseZigZagMap[0]] = dcSymbol + previousDC;
    previousDC = mcu[reverseZigZagMap[0]];

    // AC decoding
    uint8_t i = 1;
    while (i < MCUX) {
        uint8_t acInfo = decodeSymbol(stream, ac);
        if (acInfo == 0xFF) {
            return false;
        }

        uint8_t paddingZero = acInfo >> 4;
        uint8_t acLength = acInfo & 0x0F;

        if (acLength == 0) {
            if (paddingZero == ZLR) {
                // Avoid under skipping 1 bit
                i += 16;
                continue;
            } else if (paddingZero == EOB) {
                break;
            }
            std::cerr << "Decode error: Corrupted AC byte sequence" << std::endl;
            return false;
        }

        i += paddingZero;
        if (i >= MCUX) {
            std::cerr << "Decode error: AC decoding MCU subpart overflow" << std::endl;
            return false;
        }

        int16_t acSymbol = 0;
        if (!getACDCSymbol(stream, acSymbol, acLength)) {
            return false;
        }

        mcu[reverseZigZagMap[i]] = acSymbol;
        i++;
    }

    // Because of underskip logic, we check overrun a second time
    if (i > MCUX) {
        std::cerr << "Decode error: AC decoding MCU subpart overflow" << std::endl;
        return false;
    }

    return true;
}

bool decodeHuffman(std::unique_ptr<Body>& body) {
    if (!generateCode(body)) {
        return false;
    }

    // Should be ordered by class and id
    std::array<HuffmanDecodeTable, 4> decodeTables; // 2 for AC, 2 for DC

    for (auto& table : body->header->huffmanTable) {
        if (table.tableClass > 1 || table.identifier > 1) {
            std::cerr << "Decode error: number of class and identifier in DHT not supported" << std::endl;
            return false;
        }
        if (!table.completed) {
            continue;
        }
        HuffmanDecodeTable& decodeTable = decodeTables[table.tableClass * 2 + table.identifier];
        if (!fillDecodeTable(table, decodeTable)) {
            return false;
        }
    }

    auto mcus = std::move(body->mcu);

    std::array<int16_t, 4> previousDC{};
    for (std::size_t i = 0; i < mcus->mcuWidth * mcus->mcuHeight; i++) {

        if (body->header->restartInterval != 0 && i % body->header->restartInterval == 0) {
            previousDC.fill(0);
            body->data.align();
        }
        auto& mcuData = mcus->mcuData[i];
        for (uint8_t j = 0; j < body->header->numberComponents; j++) {
            if (!decodeMCU(body->data, mcuData[j], decodeTables[body->header->channels[j].huffDCId],
                           decodeTables[body->header->channels[j].huffACId + 2], previousDC[j])) {
                body->mcu = std::move(mcus);
                std::cerr << "Huffman decoding error. Stopping early." << std::endl;
                return false;
            }
        }
    }

    mcus->isValid = true;
    body->mcu = std::move(mcus);

    return true;
}