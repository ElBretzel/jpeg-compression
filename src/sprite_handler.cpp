#include "sprite_handler.hpp"

std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textureMap;
std::unordered_map<SpriteContainer, std::unique_ptr<sf::Sprite>, SpriteContainerHash> spriteMap;
int sprite_counter = 0;

int loadTexture(const std::string& texturePath) {
    if ((texturePath.empty())) {
        std::cerr << "Texture path is empty!" << std::endl;
        return -1;
    }

    if (textureMap.find(texturePath) != textureMap.end()) {
        return 0;
    }

    auto texture = std::make_unique<sf::Texture>();
    if (!texture->loadFromFile(texturePath)) {
        std::cerr << "Failed to load texture from: " << texturePath << std::endl;
        return -1;
    }

    textureMap[texturePath] = std::move(texture);
    return 0;
}

int addSprite(const std::string& texturePath, sf::Vector2f position) {
    if (loadTexture(texturePath)) {
        return -1;
    }

    SpriteContainer spriteContainer(sprite_counter++, texturePath);

    auto& texture = textureMap[texturePath];

    auto sprite = std::make_unique<sf::Sprite>(*texture);
    sprite->setTextureRect({{0, 0}, {(int)texture->getSize().x, (int)texture->getSize().y}});
    sprite->setPosition(position);
    spriteMap[spriteContainer] = std::move(sprite);

    return 0;
}

auto findSpriteById(std::size_t spriteId) {
    return std::find_if(spriteMap.begin(), spriteMap.end(), [spriteId](const auto& pair) {
        return pair.first.spriteId == spriteId;
    });
}
int moveSprite(std::size_t spriteId, sf::Vector2f newPosition) {
    auto it = findSpriteById(spriteId);
    if (it == spriteMap.end()) {
        std::cerr << "Sprite ID " << spriteId << " does not exists" << std::endl;
        return -1;
    }

    auto sprite = std::move(it->second);
    sprite->setPosition(newPosition);
    spriteMap[it->first] = std::move(sprite);

    return 0;
}
int changeSpriteTexture(std::size_t spriteId, const std::string& newTexturePath) {
    auto it = findSpriteById(spriteId);
    if (it == spriteMap.end()) {
        std::cerr << "Sprite ID " << spriteId << " does not exists" << std::endl;
        return -1;
    }

    if (loadTexture(newTexturePath)) {
        return -1;
    }

    auto sprite = std::move(it->second);

    spriteMap.erase(it);

    auto& texture = textureMap[newTexturePath];
    sprite->setTexture(*texture, true);
    sprite->setTextureRect({{0, 0}, {(int)texture->getSize().x, (int)texture->getSize().y}});

    SpriteContainer spriteContainer(it->first.spriteId, newTexturePath);
    spriteMap[spriteContainer] = std::move(sprite);

    return 0;
}

int removeSprite(std::size_t spriteId) {
    auto it = findSpriteById(spriteId);
    if (it == spriteMap.end()) {
        std::cerr << "Sprite ID " << spriteId << " does not exists" << std::endl;
        return -1;
    }

    spriteMap.erase(it);
}