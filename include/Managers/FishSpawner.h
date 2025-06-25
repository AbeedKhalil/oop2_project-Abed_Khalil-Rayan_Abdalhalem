#pragma once

#include "GenericFish.h"
#include "GenericSpawner.h"
#include <unordered_map>
#include <memory>
#include <random>

namespace FishGame
{
    // Fish spawning configuration
    struct SpawnConfig
    {
        float spawnRate;     // Fish per second
        float minY;          // Minimum Y position
        float maxY;          // Maximum Y position
    };

    class SpriteManager;

    class FishSpawner
    {
    public:
        FishSpawner(const sf::Vector2u& windowSize, SpriteManager& spriteManager);
        virtual ~FishSpawner() = default;

        // Virtual update for derived classes
        virtual void update(sf::Time deltaTime, int currentLevel);

        // Accessors
        std::vector<std::unique_ptr<Entity>>& getSpawnedFish() { return m_spawnedFish; }
        void clearSpawnedFish() { m_spawnedFish.clear(); }

        // Level configuration
        void setLevel(int level);

    protected:  // Changed to protected for inheritance
        // Methods accessible to derived classes
        void updateSpawners(sf::Time deltaTime);
        void configureSpawnersForLevel(int level);

        // Data members accessible to derived classes
        sf::Vector2u m_windowSize;
        std::vector<std::unique_ptr<Entity>> m_spawnedFish;

        // Spawners
        GenericSpawner<SmallFish> m_smallSpawner;
        GenericSpawner<MediumFish> m_mediumSpawner;
        GenericSpawner<LargeFish> m_largeSpawner;

        // Configuration maps
        std::unordered_map<int, SpawnConfig> m_smallFishConfig;
        std::unordered_map<int, SpawnConfig> m_mediumFishConfig;
        std::unordered_map<int, SpawnConfig> m_largeFishConfig;

        int m_currentLevel;
        std::mt19937 m_randomEngine;

        SpriteManager* m_spriteManager;
    };
}
