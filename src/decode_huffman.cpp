#include "decode_huffman.hpp"

bool generateCode(HuffmanTable& table) {

    uint16_t code = 0;
    for (auto& data : table.huffData) {
        data.huffCode.clear();
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

    return true;
}

// Order Huffman codes and symbols for one Huffman table
bool fillDecodeTable(const HuffmanTable& table, HuffmanDecodeTable& decodeTable) {
    decodeTable.fill({});
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