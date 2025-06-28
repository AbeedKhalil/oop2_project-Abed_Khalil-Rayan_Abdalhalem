#pragma once

#include "Fish.h"
#include "GenericFish.h"
#include "SpecialFish.h"
#include <memory>
#include <type_traits>

namespace FishGame
{
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
            else if constexpr (std::is_same_v<FishType, PoisonFish>)
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

    };

    // Note: SchoolingFishFactory is defined in SchoolingSystem.h to avoid circular dependencies
}
