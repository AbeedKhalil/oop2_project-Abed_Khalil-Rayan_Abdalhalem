#include "EnhancedFishSpawner.h"
#include "GameConstants.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace FishGame
{
    // Template implementation for ConfiguredFishFactory
    template<typename FishType>
    std::vector<std::unique_ptr<FishType>> ConfiguredFishFactory<FishType>::createFormation(
        const ConfigType& config,
        const sf::Vector2f& basePosition,
        int level)
    {
        std::vector<std::unique_ptr<FishType>> formation;
        formation.reserve(config.count);

        // Use STL algorithm to generate formation
        std::generate_n(std::back_inserter(formation), config.count,
            [&config, &basePosition, level, index = 0]() mutable -> std::unique_ptr<FishType>
            {
                auto fish = std::make_unique<FishType>(level);

                // Calculate position based on pattern
                sf::Vector2f position = calculatePosition(
                    config.pattern, basePosition, index, config.spacing);

                fish->setPosition(position);
                config.applyToFish(*fish, index);

                index++;
                return fish;
            });

        return formation;
    }

    template<typename FishType>
    sf::Vector2f ConfiguredFishFactory<FishType>::calculatePosition(
        SpawnPattern pattern,
        const sf::Vector2f& base,
        size_t index,
        float spacing)
    {
        switch (pattern)
        {
        case SpawnPattern::WaveFormation:
            return sf::Vector2f(
                base.x + index * spacing,
                base.y + std::sin(index * 0.5f) * 30.0f
            );

        case SpawnPattern::CircleFormation:
        {
            float angle = (360.0f / 8.0f) * index * Constants::DEG_TO_RAD;
            return sf::Vector2f(
                base.x + std::cos(angle) * spacing,
                base.y + std::sin(angle) * spacing
            );
        }

        case SpawnPattern::LineFormation:
            return sf::Vector2f(base.x, base.y + index * spacing);

        case SpawnPattern::EdgeRandom:
        default:
            return base;
        }
    }

    // Explicit template instantiations for commonly used fish types
    template class ConfiguredFishFactory<SmallFish>;
    template class ConfiguredFishFactory<MediumFish>;
    template class ConfiguredFishFactory<LargeFish>;
    template class ConfiguredFishFactory<Barracuda>;
    template class ConfiguredFishFactory<Pufferfish>;
    template class ConfiguredFishFactory<Angelfish>;
}
