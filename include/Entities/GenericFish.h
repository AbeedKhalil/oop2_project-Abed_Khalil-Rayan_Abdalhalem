#pragma once

#include "Fish.h"
#include "GameConstants.h"
#include "SpriteManager.h"

namespace FishGame
{
    // Fish configuration traits
    template<FishSize Size>
    struct FishTraits
    {
        static constexpr float speed = 0.0f;
        static constexpr float radius = 0.0f;
        static constexpr float outlineThickness = 1.0f;
        static sf::Color fillColor;
        static sf::Color outlineColor;
    };

    // Template specializations for each fish size
    template<>
    struct FishTraits<FishSize::Small>
    {
        static constexpr float speed = Constants::SMALL_FISH_SPEED;
        static constexpr float radius = Constants::SMALL_FISH_RADIUS;
        static constexpr float outlineThickness = 1.0f;
        static inline sf::Color fillColor = Constants::SMALL_FISH_COLOR;
        static inline sf::Color outlineColor = Constants::SMALL_FISH_OUTLINE;
    };

    template<>
    struct FishTraits<FishSize::Medium>
    {
        static constexpr float speed = Constants::MEDIUM_FISH_SPEED;
        static constexpr float radius = Constants::MEDIUM_FISH_RADIUS;
        static constexpr float outlineThickness = 1.5f;
        static inline sf::Color fillColor = Constants::MEDIUM_FISH_COLOR;
        static inline sf::Color outlineColor = Constants::MEDIUM_FISH_OUTLINE;
    };

    template<>
    struct FishTraits<FishSize::Large>
    {
        static constexpr float speed = Constants::LARGE_FISH_SPEED;
        static constexpr float radius = Constants::LARGE_FISH_RADIUS;
        static constexpr float outlineThickness = 2.0f;
        static inline sf::Color fillColor = Constants::LARGE_FISH_COLOR;
        static inline sf::Color outlineColor = Constants::LARGE_FISH_OUTLINE;
    };

    // Generic fish template
    template<FishSize Size>
    class GenericFish : public Fish
    {
    public:
        explicit GenericFish(int currentLevel = 1)
            : Fish(Size, FishTraits<Size>::speed, currentLevel)
        {
        }

        ~GenericFish() override = default;

        EntityType getType() const override
        {
            if constexpr (Size == FishSize::Small)
                return EntityType::SmallFish;
            else if constexpr (Size == FishSize::Medium)
                return EntityType::MediumFish;
            else
                return EntityType::LargeFish;
        }

        // Override to get specific texture
        TextureID getTextureID() const override
        {
            if constexpr (Size == FishSize::Small)
                return TextureID::SmallFish;
            else if constexpr (Size == FishSize::Medium)
                return TextureID::MediumFish;
            else
                return TextureID::LargeFish;
        }

    protected:
    };

    // Type aliases for convenience and backward compatibility
    using SmallFish = GenericFish<FishSize::Small>;
    using MediumFish = GenericFish<FishSize::Medium>;
    using LargeFish = GenericFish<FishSize::Large>;
}
