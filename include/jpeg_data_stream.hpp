#pragma once

#include <fstream>
#include <iostream>

#include "jpeg_def.hpp"

class JpegDataStream {
  public:
    JpegDataStream(const std::string& streamPath) : file(streamPath, std::ios::binary) {
        if (streamPath.empty()) {
            std::cerr << "Can not check validity of jpeg file: " << "path is empty" << std::endl;
        }
        if (!streamPath.ends_with(".jpg") && !streamPath.ends_with(".jpeg")) {
            std::cerr << "Can not check validity of jpeg file: " << "file has incorrect format" << std::endl;
        }
        if (!file.is_open()) {
            std::cerr << "Failed to open given file..." << std::endl;
        }
        currentByte = file.get();
    }
    ~JpegDataStream() {
        file.close();
    }

    uint8_t readBit(bool scan = false);
    uint8_t readByte(bool scan = false);
    uint64_t readBits(uint8_t length, bool scan = false);
    uint8_t peekBit() const;
    void align();
    bool isEOF();

  private:
    std::ifstream file;
    std::uint8_t bitPos;
    uint8_t currentByte;
};