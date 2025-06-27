#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>

struct SpriteContainer {
    std::size_t spriteId;
    std::string texturePath;

    bool operator==(const SpriteContainer& other) const {
        return spriteId == other.spriteId && texturePath == other.texturePath;
    }

    SpriteContainer(std::size_t spriteId, std::string texturePath) : spriteId(spriteId), texturePath(texturePath) {}
};
struct SpriteContainerHash {
    std::size_t operator()(const SpriteContainer& other) const noexcept {
        std::size_t h1 = std::hash<std::size_t>{}(other.spriteId);
        std::size_t h2 = std::hash<std::string>{}(other.texturePath);
        return h1 ^ (h2 << 1);
    }
};

int loadTexture(const std::string& texturePath);
int addSprite(const std::string& texturePath, sf::Vector2f position);
SpriteContainer findSprite(std::size_t spriteId);
int moveSprite(std::size_t spriteId, sf::Vector2f newPosition);
int changeSpriteTexture(std::size_t spriteId, const std::string& newTexturePath);

extern std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textureMap;
extern std::unordered_map<SpriteContainer, std::unique_ptr<sf::Sprite>, SpriteContainerHash> spriteMap;
extern int sprite_counter;