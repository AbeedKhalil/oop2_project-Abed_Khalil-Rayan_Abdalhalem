#include "SpriteManager.h"
#include "Entity.h"
#include "Fish.h"
#include "Player.h"
#include <stdexcept>
#include <algorithm>

namespace FishGame
{
    // Static texture file mappings
    const SpriteManager::TextureMap SpriteManager::s_textureFiles = {
        // Player textures
        {TextureID::PlayerSmall, "PlayerFish.png"},
        {TextureID::PlayerMedium, "PlayerFish.png"},
        {TextureID::PlayerLarge, "PlayerFish.png"},

        // Enemy fish
        {TextureID::SmallFish, "SmallFish.png"},
        {TextureID::MediumFish, "MediumFish.png"},
        {TextureID::LargeFish, "LargeFish.png"},

        // Special fish
        {TextureID::Barracuda, "Barracuda.png"},
        {TextureID::Pufferfish, "Pufferfish.png"},
        {TextureID::PufferfishInflated, "Pufferfish.png"},
        {TextureID::Angelfish, "Angelfish.png"},
        {TextureID::PoisonFish, "PoisonFish.png"},

        // Bonus items
        {TextureID::Starfish, "StarFish.png"},
        {TextureID::PearlOysterClosed, "Oyster.png"},
        {TextureID::PearlOysterOpen, "Oyster.png"},
        {TextureID::WhitePearl, "WhitePearl.png"},
        {TextureID::BlackPearl, "BlackPearl.png"},

        // Hazards
        {TextureID::Bomb, "Bomb.png"},
        {TextureID::Jellyfish, "Jellyfish.png"},

        // Power-ups
        {TextureID::PowerUpSpeedBoost, "PowerupSpeed.png"},
        {TextureID::PowerUpAddTime, "PowerupTime.png"},
        {TextureID::PowerUpExtraLife, "PowerupLife.png"},

        // Environment
        {TextureID::Background, "Background1.png"},
        {TextureID::GameTitle, "GameTitle.png"},

        // Menu sprites
        {TextureID::NewGame, "NewGame.png"},
        {TextureID::NewGameHover, "NewGameHover.png"},
        {TextureID::GameOptions, "GameOptions.png"},
        {TextureID::GameOptionsHover, "GameOptionsHover.png"},
        {TextureID::Exit, "Exit.png"},
        {TextureID::ExitHover, "ExitHover.png"},
        { TextureID::Intro1, "Intro1.png" },
        {TextureID::Intro2, "Intro2.png"}
    };

    SpriteManager::SpriteManager(ResourceHolder<sf::Texture, TextureID>& textureHolder)
        : m_textureHolder(textureHolder)
        , m_scaleConfig()
    {
    }

    bool SpriteManager::loadTextures(const std::string& assetPath)
    {
        bool allLoaded = true;

        // Use STL algorithm to load all textures
        std::for_each(s_textureFiles.begin(), s_textureFiles.end(),
            [this, &assetPath, &allLoaded](const auto& pair) {
                const auto& [id, filename] = pair;
                std::string fullPath = filename;

                if (!m_textureHolder.load(id, fullPath))
                {
                    // Log error but continue loading other textures
                    allLoaded = false;
                }
            });

        return allLoaded;
    }

    const sf::Texture& SpriteManager::getTexture(TextureID id) const
    {
        return m_textureHolder.get(id);
    }

    template<typename EntityType>
    std::unique_ptr<SpriteComponent<EntityType>> SpriteManager::createSpriteComponent(
        EntityType* owner, TextureID textureId)
    {
        auto component = std::make_unique<SpriteComponent<EntityType>>(owner);

        // Set texture
        component->setTexture(getTexture(textureId));

        // Apply default configuration
        SpriteConfig<EntityType> config = getSpriteConfig<EntityType>(textureId);
        component->configure(config);

        return component;
    }

    template<typename EntityType>
    SpriteConfig<EntityType> SpriteManager::getSpriteConfig(TextureID textureId, FishSize size)
    {
        SpriteConfig<EntityType> config;
        config.textureName = s_textureFiles.at(textureId);

        // Set scale based on entity type and size
        if constexpr (std::is_base_of_v<Fish, EntityType>)
        {
            config.scaleMultiplier = getScaleForSize(size);
        }
        else if constexpr (std::is_same_v<Player, EntityType>)
        {
            config.scaleMultiplier = getScaleForSize(size);
        }
        else
        {
            config.scaleMultiplier = 1.0f;
        }

        // Special configurations for specific entities
        switch (textureId)
        {
        case TextureID::Pufferfish:
        case TextureID::PufferfishInflated:
            config.baseSize = sf::Vector2f(50.0f, 50.0f);
            break;

        case TextureID::SmallFish:
        case TextureID::PoisonFish:
        case TextureID::Angelfish:
            config.scaleMultiplier = getScaleForSize(size) * 1.5f;
            break;

        case TextureID::Jellyfish:
            config.baseSize = sf::Vector2f(10.0f, 30.0f);
            config.scaleMultiplier = 0.7f;
            break;

        case TextureID::Barracuda:
            config.scaleMultiplier = getScaleForSize(size) * 1.3f;
            break;

        case TextureID::Starfish:
            config.baseSize = sf::Vector2f(50.0f, 50.0f);
            config.rotationOffset = 0.0f; // Will be animated
            config.maintainAspectRatio = false;
            break;

        default:
            config.baseSize = sf::Vector2f(60.0f, 40.0f); // Default fish size
            break;
        }

        config.maintainAspectRatio = textureId != TextureID::Starfish;
        return config;
    }

    float SpriteManager::getScaleForSize(FishSize size) const
    {
        switch (size)
        {
        case FishSize::Small:
            return m_scaleConfig.small;
        case FishSize::Medium:
            return m_scaleConfig.medium;
        case FishSize::Large:
            return m_scaleConfig.large;
        default:
            return 1.0f;
        }
    }

    // Explicit template instantiations
    template std::unique_ptr<SpriteComponent<Entity>>
        SpriteManager::createSpriteComponent(Entity*, TextureID);
    template std::unique_ptr<SpriteComponent<Fish>>
        SpriteManager::createSpriteComponent(Fish*, TextureID);
    template std::unique_ptr<SpriteComponent<Player>>
        SpriteManager::createSpriteComponent(Player*, TextureID);

    template SpriteConfig<Entity> SpriteManager::getSpriteConfig(TextureID, FishSize);
    template SpriteConfig<Fish> SpriteManager::getSpriteConfig(TextureID, FishSize);
    template SpriteConfig<Player> SpriteManager::getSpriteConfig(TextureID, FishSize);
}