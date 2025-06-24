#pragma once

#include "SpriteComponent.h"
#include "ResourceHolder.h"
#include "Fish.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace FishGame
{
    // Forward declarations
    class Entity;
    class Fish;
    class Player;

    // Texture identifiers extended for sprites
    enum class TextureID
    {
        // Player
        PlayerSmall,
        PlayerMedium,
        PlayerLarge,

        // Enemy Fish
        SmallFish,
        MediumFish,
        LargeFish,

        // Special Fish
        Barracuda,
        Pufferfish,
        PufferfishInflated,
        Angelfish,
        PoisonFish,

        // Bonus Items
        Starfish,
        PearlOysterClosed,
        PearlOysterOpen,
        WhitePearl,
        BlackPearl,

        // Hazards
        Bomb,
        Jellyfish,

        // Power-ups
        PowerUpSpeedBoost,
        PowerUpAddTime,
        PowerUpExtraLife,

        // Environment
        Background1,
        Background2,
        Background3,
        Background4,
        Background5,
        Background6,
        // legacy identifier kept for compatibility
        Background = Background1,
        GameTitle
        , NewGame
        , NewGameHover
        , GameOptions
        , GameOptionsHover
        , Exit
        , ExitHover
        , Intro1
        , Intro2
    };

    // Sprite scale configurations
    struct SpriteScaleConfig
    {
        float small = 1.0f;
        float medium = 4.0f;
        float large = 8.0f;
    };

    class SpriteManager
    {
    public:
        using TextureMap = std::unordered_map<TextureID, std::string>;

        explicit SpriteManager(ResourceHolder<sf::Texture, TextureID>& textureHolder);
        ~SpriteManager() = default;

        // Delete copy operations
        SpriteManager(const SpriteManager&) = delete;
        SpriteManager& operator=(const SpriteManager&) = delete;

        // Load all textures
        void loadTextures(const std::string& assetPath);

        // Get texture for entity
        const sf::Texture& getTexture(TextureID id) const;

        // Create sprite component for entity
        template<typename EntityType>
        std::unique_ptr<SpriteComponent<EntityType>> createSpriteComponent(
            EntityType* owner, TextureID textureId);

        // Get sprite configuration
        template<typename EntityType>
        SpriteConfig<EntityType> getSpriteConfig(TextureID textureId, FishSize size = FishSize::Small);

        // Scale configurations
        void setScaleConfig(const SpriteScaleConfig& config) { m_scaleConfig = config; }
        const SpriteScaleConfig& getScaleConfig() const { return m_scaleConfig; }

    private:
        ResourceHolder<sf::Texture, TextureID>& m_textureHolder;
        SpriteScaleConfig m_scaleConfig;

        // Texture file mappings
        static const TextureMap s_textureFiles;

        // Helper to determine scale based on size
        float getScaleForSize(FishSize size) const;
    };
}
