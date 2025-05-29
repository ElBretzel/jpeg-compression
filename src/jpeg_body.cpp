#include "jpeg_body.hpp"

std::unique_ptr<Body> scanBody(const std::string& filePath, std::unique_ptr<Header>& header) {
    auto body = std::make_unique<Body>();
    body->header = std::move(header);

    if (!body->header->isValid) {
        std::cerr << "Body error: Won't read body until header is valid..." << std::endl;
        return body;
    }
    return body;
}