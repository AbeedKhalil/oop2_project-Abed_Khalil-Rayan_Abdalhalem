#pragma once
#include "ISpriteManager.h"
#include <memory>

namespace FishGame {
class DefaultSpriteManager : public ISpriteManager {
public:
    DefaultSpriteManager();
    ~DefaultSpriteManager() override = default;

    void loadTextures(const std::string& assetPath) override;
    const sf::Texture& getTexture(TextureID id) const override;
    void setScaleConfig(const SpriteScaleConfig& config) override;
    const SpriteScaleConfig& getScaleConfig() const override;

    SpriteManager& getRawManager() override { return *m_manager; }
    const SpriteManager& getRawManager() const override { return *m_manager; }

protected:
    SpriteManager& getImpl() override { return *m_manager; }
    const SpriteManager& getImpl() const override { return *m_manager; }

private:
    std::unique_ptr<ResourceHolder<sf::Texture, TextureID>> m_textures;
    std::unique_ptr<SpriteManager> m_manager;
};
} // namespace FishGame
