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
                std::cerr << "Can not generate code: " << "There is too many symbols to generate code of fixed length"
                          << std::endl;
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
        HuffmanDecodeTable& decodeTable = decodeTables[table.tableClass * 2 + table.identifier];
        if (!fillDecodeTable(table, decodeTable)) {
            return false;
        }
    }

    return true;
}