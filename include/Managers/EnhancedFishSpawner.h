#pragma once

#include "FishSpawner.h"
#include "SpecialFish.h"
#include "SchoolingSystem.h"
#include <type_traits>
#include <unordered_map>
#include <string>

namespace FishGame
{
    // Forward declaration
    class SchoolingSystem;

    // Special fish spawn configuration
    struct SpecialFishConfig
    {
        float barracudaSpawnRate = 0.1f;
        float pufferfishSpawnRate = 0.15f;
        float angelfishSpawnRate = 0.2f;
        float poisonFishSpawnRate = 0.12f;
        float schoolSpawnChance = 0.3f;
    };

    class EnhancedFishSpawner : public FishSpawner
    {
    public:
        EnhancedFishSpawner(const sf::Vector2u& windowSize, SpriteManager& spriteManager);
        ~EnhancedFishSpawner() = default;

        // Override update to include special fish
        void update(sf::Time deltaTime, int currentLevel) override;

        // Setters
        void setSchoolingSystem(SchoolingSystem* schoolingSystem)
        {
            m_schoolingSystem = schoolingSystem;
        }

        void setSpecialFishConfig(const SpecialFishConfig& config)
        {
            m_specialConfig = config;
        }

    private:
        // Template methods
        template<typename SpecialFishType>
        void spawnSpecialFish(float spawnRate, sf::Time deltaTime);

        template<typename FishType>
        void spawnSchool(size_t count);

        void configureSpecialSpawners(int level);

    private:
        // Special fish spawners
        GenericSpawner<Barracuda> m_barracudaSpawner;
        GenericSpawner<Pufferfish> m_pufferfishSpawner;
        GenericSpawner<Angelfish> m_angelfishSpawner;
        GenericSpawner<PoisonFish> m_poisonFishSpawner;

        // Configuration
        SpecialFishConfig m_specialConfig;
        SchoolingSystem* m_schoolingSystem;

        // Spawn timers
        std::unordered_map<std::string, sf::Time> m_specialSpawnTimers;

        // Random distributions
        std::uniform_real_distribution<float> m_schoolChanceDist;
        std::uniform_int_distribution<int> m_schoolSizeDist;
    };

    // Spawning patterns
    enum class SpawnPattern
    {
        EdgeRandom,
        WaveFormation,
        CircleFormation,
        LineFormation
    };

    // Advanced spawning configuration
    template<typename FishType>
    struct AdvancedSpawnConfig
    {
        SpawnPattern pattern = SpawnPattern::EdgeRandom;
        size_t count = 1;
        float spacing = 50.0f;
        MovementPattern movementPattern = MovementPattern::Linear;

        void applyToFish(FishType& fish, [[maybe_unused]] size_t index) const
        {
            if constexpr (std::is_base_of_v<AdvancedFish, FishType>)
            {
                static_cast<AdvancedFish&>(fish).setMovementPattern(movementPattern);
            }
        }
    };

    // Fish formation factory
    template<typename FishType>
    class ConfiguredFishFactory
    {
    public:
        using ConfigType = AdvancedSpawnConfig<FishType>;

        static std::vector<std::unique_ptr<FishType>> createFormation(
            const ConfigType& config,
            const sf::Vector2f& basePosition,
            int level);

    private:
        static sf::Vector2f calculatePosition(
            SpawnPattern pattern,
            const sf::Vector2f& base,
            size_t index,
            float spacing);
    };
}
