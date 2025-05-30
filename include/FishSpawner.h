#pragma once

#include "Fish.h"
#include "GenericFish.h"
#include "GenericSpawner.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <random>
#include <functional>
#include <vector>
#include <map>

namespace FishGame
{
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
        void updateSpawners(sf::Time deltaTime);
        void configureSpawnersForLevel(int level);

    private:
        sf::Vector2u m_windowSize;
        std::vector<std::unique_ptr<Entity>> m_spawnedFish;

        // Generic spawners for each fish type
        GenericSpawner<SmallFish> m_smallSpawner;
        GenericSpawner<MediumFish> m_mediumSpawner;
        GenericSpawner<LargeFish> m_largeSpawner;

        // Level-based spawn configurations
        std::map<int, SpawnConfig> m_smallFishConfig;
        std::map<int, SpawnConfig> m_mediumFishConfig;
        std::map<int, SpawnConfig> m_largeFishConfig;

        int m_currentLevel;
        std::mt19937 m_randomEngine;
    };
}