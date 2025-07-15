#pragma once
#include "SpriteManager.h"

namespace FishGame {
class ISpriteManager {
public:
    virtual ~ISpriteManager() = default;
    virtual void loadTextures(const std::string& assetPath) = 0;
    virtual const sf::Texture& getTexture(TextureID id) const = 0;
    virtual void setScaleConfig(const SpriteScaleConfig& config) = 0;
    virtual const SpriteScaleConfig& getScaleConfig() const = 0;

    virtual SpriteManager& getRawManager() = 0;
    virtual const SpriteManager& getRawManager() const = 0;

    template <typename EntityType>
    std::unique_ptr<SpriteComponent<EntityType>> createSpriteComponent(EntityType* owner, TextureID textureId) {
        return getImpl().createSpriteComponent(owner, textureId);
    }

    template <typename EntityType>
    SpriteConfig<EntityType> getSpriteConfig(TextureID textureId, FishSize size = FishSize::Small) {
        return getImpl().getSpriteConfig<EntityType>(textureId, size);
    }

protected:
    virtual SpriteManager& getImpl() = 0;
    virtual const SpriteManager& getImpl() const = 0;
};
} // namespace FishGame
