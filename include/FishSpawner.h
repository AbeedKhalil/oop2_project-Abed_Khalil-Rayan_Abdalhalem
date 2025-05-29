// FishSpawner.h
#pragma once

#include "Fish.h"
#include "SmallFish.h"
#include "MediumFish.h"
#include "LargeFish.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <random>
#include <functional>
#include <vector>
#include <map>

namespace FishGame
{
    // Template class for spawning different fish types
    template<typename T>
    class FishFactory
    {
    public:
        static std::unique_ptr<T> create(int currentLevel)
        {
            return std::make_unique<T>(currentLevel);
        }
    };

    // Fish spawning configuration
    struct SpawnConfig
    {
        float spawnRate;     // Fish per second
        float minY;          // Minimum Y position
        float maxY;          // Maximum Y position
        bool fromLeft;       // Spawn direction
    };

    class FishSpawner
    {
    public:
        explicit FishSpawner(const sf::Vector2u& windowSize);
        ~FishSpawner() = default;

        // Update spawning logic
        void update(sf::Time deltaTime, int currentLevel);

        // Get spawned fish
        std::vector<std::unique_ptr<Entity>>& getSpawnedFish() { return m_spawnedFish; }
        void clearSpawnedFish() { m_spawnedFish.clear(); }

        // Level-based configuration
        void setLevel(int level);

    private:
        // Template method for spawning specific fish type
        template<typename FishType>
        void spawnFish(bool fromLeft);

        // Spawn timing control
        void updateSpawnTimers(sf::Time deltaTime);
        bool shouldSpawn(sf::Time& timer, float spawnRate, sf::Time deltaTime);

    private:
        sf::Vector2u m_windowSize;
        std::vector<std::unique_ptr<Entity>> m_spawnedFish;

        // Random number generation
        std::mt19937 m_randomEngine;
        std::uniform_real_distribution<float> m_yDistribution;
        std::uniform_real_distribution<float> m_spawnChance;

        // Spawn timers
        sf::Time m_smallFishTimer;
        sf::Time m_mediumFishTimer;
        sf::Time m_largeFishTimer;

        // Level-based spawn rates (fish per second)
        std::map<int, SpawnConfig> m_smallFishConfig;
        std::map<int, SpawnConfig> m_mediumFishConfig;
        std::map<int, SpawnConfig> m_largeFishConfig;

        int m_currentLevel;
    };
}