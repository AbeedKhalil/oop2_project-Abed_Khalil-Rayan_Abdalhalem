#include "SpriteManager.h"
#include "Entity.h"
#include "Fish.h"
#include "Player.h"
#include <algorithm>
#include <future>
#include <vector>
#include "GameExceptions.h"

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
        {TextureID::Background1, "Background1.png"},
        {TextureID::Background2, "Background2.png"},
        {TextureID::Background3, "Background3.png"},
        {TextureID::Background4, "Background4.png"},
        {TextureID::Background5, "Background5.png"},
        {TextureID::Background6, "Background6.png"},
        {TextureID::GameTitle, "GameTitle.png"},

        // Menu sprites
        {TextureID::NewGame, "NewGame.png"},
        {TextureID::NewGameHover, "NewGameHover.png"},
        {TextureID::GameOptions, "GameOptions.png"},
        {TextureID::GameOptionsHover, "GameOptionsHover.png"},
        {TextureID::Exit, "Exit.png"},
        {TextureID::ExitHover, "ExitHover.png"},
        { TextureID::Intro1, "Intro1.png" },
        {TextureID::Intro2, "Intro2.png"},
        {TextureID::StageIntro, "StageIntro.png"},
        {TextureID::Button, "Button.png"},
        {TextureID::ButtonHover, "ButtonHover.png"}
    };

    SpriteManager::SpriteManager(ResourceHolder<sf::Texture, TextureID>& textureHolder)
        : m_textureHolder(textureHolder)
        , m_scaleConfig()
    {
    }

void SpriteManager::loadTextures(const std::string& assetPath)
{
        m_textureHolder.reserve(s_textureFiles.size());

        // Load textures in parallel using std::async
        std::vector<std::future<std::pair<TextureID, std::unique_ptr<sf::Texture>>>> futures;
        futures.reserve(s_textureFiles.size());

        for (const auto& [id, filename] : s_textureFiles)
        {
            futures.emplace_back(std::async(std::launch::async,
                [&, id, filename]() {
                    std::string fullPath = assetPath.empty() ? filename
                        : assetPath + "/" + filename;
                    auto tex = std::make_unique<sf::Texture>();
                    if (!tex->loadFromFile(fullPath))
                    {
                        throw ResourceLoadException("Failed to load texture: " + fullPath);
                    }
                    return std::make_pair(id, std::move(tex));
                }));
        }

        for (auto& fut : futures)
        {
            auto result = fut.get();
            m_textureHolder.insert(result.first, std::move(result.second));
        }
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

        case TextureID::PowerUpExtraLife:
        case TextureID::PowerUpSpeedBoost:
        case TextureID::PowerUpAddTime:
            config.scaleMultiplier = 0.7f; //scale the power ups
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
