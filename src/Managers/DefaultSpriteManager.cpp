#include "DefaultSpriteManager.h"

namespace FishGame {

DefaultSpriteManager::DefaultSpriteManager()
    : m_textures(std::make_unique<ResourceHolder<sf::Texture, TextureID>>()),
      m_manager(std::make_unique<SpriteManager>(*m_textures)) {}

void DefaultSpriteManager::loadTextures(const std::string& assetPath) {
    m_manager->loadTextures(assetPath);
}

const sf::Texture& DefaultSpriteManager::getTexture(TextureID id) const {
    return m_manager->getTexture(id);
}

void DefaultSpriteManager::setScaleConfig(const SpriteScaleConfig& config) {
    m_manager->setScaleConfig(config);
}

const SpriteScaleConfig& DefaultSpriteManager::getScaleConfig() const {
    return m_manager->getScaleConfig();
}

} // namespace FishGame
