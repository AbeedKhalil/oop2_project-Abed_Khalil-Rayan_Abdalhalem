#pragma once

#include "Fish.h"
#include "SpriteManager.h"
#include <type_traits>

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
        static constexpr float speed = 150.0f;
        static constexpr float radius = 15.0f;
        static constexpr float outlineThickness = 1.0f;
        static inline sf::Color fillColor = sf::Color::Green;
        static inline sf::Color outlineColor = sf::Color(0, 100, 0);
    };

    template<>
    struct FishTraits<FishSize::Medium>
    {
        static constexpr float speed = 120.0f;
        static constexpr float radius = 25.0f;
        static constexpr float outlineThickness = 1.5f;
        static inline sf::Color fillColor = sf::Color::Blue;
        static inline sf::Color outlineColor = sf::Color(0, 0, 100);
    };

    template<>
    struct FishTraits<FishSize::Large>
    {
        static constexpr float speed = 90.0f;
        static constexpr float radius = 35.0f;
        static constexpr float outlineThickness = 2.0f;
        static inline sf::Color fillColor = sf::Color::Red;
        static inline sf::Color outlineColor = sf::Color(100, 0, 0);
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