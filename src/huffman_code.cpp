#include "huffman_code.hpp"

uint8_t decodeSymbol(JpegDataStream& stream, const HuffmanDecodeTable& decodeTable) {

    uint16_t code = 0;
    // codeLength is shifted by 1
    for (uint8_t codeLength = 0; codeLength < DHTBITS; codeLength++) {
        uint8_t b = stream.readBit(true);
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

    uint16_t s = stream.readBits(length, true);
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
               int16_t& previousDC, uint8_t type, Progressive& progressiveInfo, std::size_t& band) {

    if (type == SOF0) {
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
    } else {
        // SOF2

        // See G.1.1.2.1 and G.1.1.2.2

        // DC
        if (progressiveInfo.successiveBitHigh == 0 && progressiveInfo.startOfSpectral == 0) {
            // First visit
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

            previousDC = dcSymbol + previousDC;
            mcu[reverseZigZagMap[0]] = previousDC << progressiveInfo.successiveBitLow;
            return true;
        } else if (progressiveInfo.successiveBitHigh != 0 && progressiveInfo.startOfSpectral == 0) {
            std::cerr << "Decode Huffman progressive: DC refinement not implemented yet" << std::endl;
            // See important note in AC refinement
            return false;
        }
        // AC
        else if (progressiveInfo.successiveBitHigh == 0 && progressiveInfo.startOfSpectral != 0) {
            // First visit

            if (band > 0) {
                band--;
                return true;
            }

            uint8_t i = progressiveInfo.startOfSpectral;
            while (i <= progressiveInfo.endOfSpectral) {
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
                    } else {
                        band = (1 << paddingZero) - 1;
                        auto extension = stream.readBits(paddingZero, true);
                        if (extension == 0xFFFF) {
                            std::cerr << "Decode Huffman progressive: extension field corrupted" << std::endl;
                        }
                        band += extension;
                        break;
                    }
                    std::cerr << "Decode error: Corrupted AC byte sequence" << std::endl;
                    return false;
                }

                i += paddingZero;
                if (i > progressiveInfo.endOfSpectral) {
                    std::cerr << "Decode error: AC decoding MCU subpart overflow" << std::endl;
                    return false;
                }

                int16_t acSymbol = 0;
                if (!getACDCSymbol(stream, acSymbol, acLength)) {
                    return false;
                }

                mcu[reverseZigZagMap[i]] = acSymbol << progressiveInfo.successiveBitLow;
                i++;
            }

            // Because of underskip logic, we check overrun a second time
            if (i > progressiveInfo.endOfSpectral + 1) {
                std::cerr << "Decode error: AC decoding MCU subpart overflow" << std::endl;
                return false;
            }
            return true;
        } else if (progressiveInfo.successiveBitHigh != 0 && progressiveInfo.startOfSpectral != 0) {

            // IMPORTANT NOTE:
            // The whole AC refinement is not implemented
            // It is because the code is extremely undocumented on official JPEG documentation
            // ITU-T T.81 Annex G Fig G.7 (Only encoding is "explained")
            // So only the first visit is implemented and not all progressive jpeg can be decoded
            std::cerr << "Decode Huffman progressive: AC refinement not implemented yet" << std::endl;
            return false;
        }
        return true;
    }
}

bool decodeHuffman(std::unique_ptr<Body>& body) {

    auto mcus = std::move(body->mcu);
    uint8_t n = body->header->type == SOF0 ? 2 : 4;

    std::array<int16_t, 4> previousDC{};
    std::size_t band = 0;
    for (std::size_t i = 0; i < mcus->mcuWidth * mcus->mcuHeight; i++) {

        if (body->header->restartInterval != 0 && i != 0 && i % body->header->restartInterval == 0) {
            previousDC.fill(0);
            body->data.align();
            band = 0;
        }
        auto& mcuData = mcus->mcuData[i];
        for (uint8_t j = 0; j < body->header->numberComponents; j++) {
            if (body->header->channels[j].scan_completed) {
                if (!decodeMCU(body->data, mcuData[j], body->header->decodeTables[body->header->channels[j].huffDCId],
                               body->header->decodeTables[body->header->channels[j].huffACId + n], previousDC[j],
                               body->header->type, body->header->progressiveInfo, band)) {
                    body->mcu = std::move(mcus);
                    std::cerr << "Huffman decoding error. Stopping early." << std::endl;
                    return false;
                }
            }
        }
    }

    mcus->isValid = true;
    body->mcu = std::move(mcus);

    return true;
}