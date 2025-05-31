#pragma once

#include "Fish.h"
#include "GenericFish.h"
#include "SpecialFish.h"
#include <memory>
#include <type_traits>

namespace FishGame
{
    // Template trait for fish construction parameters
    template<typename FishType>
    struct FishConstructorTraits
    {
        static constexpr size_t paramCount = 1;
        using ParamType = int; // Default to level only
    };

    // Specializations for fish that need different parameters
    template<>
    struct FishConstructorTraits<AdvancedFish>
    {
        static constexpr size_t paramCount = 4;
        using ParamType = std::tuple<FishSize, float, int, MovementPattern>;
    };

    // Template factory for creating fish instances
    template<typename FishType>
    class FishFactory
    {
    public:
        // Factory method using if constexpr for compile-time branching
        static std::unique_ptr<FishType> create(int level)
        {
            if constexpr (std::is_same_v<FishType, SmallFish> ||
                std::is_same_v<FishType, MediumFish> ||
                std::is_same_v<FishType, LargeFish>)
            {
                return std::make_unique<FishType>(level);
            }
            else if constexpr (std::is_same_v<FishType, Barracuda> ||
                std::is_same_v<FishType, Pufferfish> ||
                std::is_same_v<FishType, Angelfish>)
            {
                return std::make_unique<FishType>(level);
            }
            else
            {
                static_assert(std::is_base_of_v<Fish, FishType>,
                    "FishFactory can only create Fish-derived types");
                return nullptr;
            }
        }

        // Create with custom attributes from another fish
        template<typename SourceFish>
        static std::unique_ptr<FishType> createFrom(const SourceFish& source, int level)
        {
            auto fish = create(level);
            if (fish)
            {
                // Copy common attributes
                fish->setPosition(source.getPosition());
                fish->setVelocity(source.getVelocity());

                if constexpr (std::is_base_of_v<Fish, SourceFish>)
                {
                    fish->setWindowBounds(source.getWindowBounds());
                }
            }
            return fish;
        }
    };

    // Template concept for schoolable fish (C++20 style, fallback for C++17)
    template<typename T>
    inline constexpr bool is_schoolable_v =
        std::is_same_v<T, SmallFish> ||
        std::is_same_v<T, MediumFish> ||
        std::is_same_v<T, Angelfish>;

    // Note: SchoolingFishFactory is defined in SchoolingSystem.h to avoid circular dependencies
}