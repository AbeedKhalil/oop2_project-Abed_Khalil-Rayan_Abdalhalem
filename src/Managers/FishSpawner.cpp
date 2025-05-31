#include "FishSpawner.h"
#include "GameConstants.h"
#include <algorithm>

namespace FishGame
{
    using namespace Constants;

    FishSpawner::FishSpawner(const sf::Vector2u& windowSize)
        : m_windowSize(windowSize)
        , m_spawnedFish()
        , m_smallSpawner()
        , m_mediumSpawner()
        , m_largeSpawner()
        , m_currentLevel(1)
        , m_randomEngine(std::random_device{}())
    {
        // Initialize spawn configurations for each level
        // Reduced small fish, maintained medium and large fish rates

        // Level 1: Easier difficulty
        m_smallFishConfig[1] = { 0.0001f, 100.0f, m_windowSize.y - 100.0f, true };
        m_mediumFishConfig[1] = { 1.2f, 150.0f, m_windowSize.y - 150.0f, false }; // Keep as is
        m_largeFishConfig[1] = { 0.8f, 200.0f, m_windowSize.y - 200.0f, true };   // Keep as is

        // Level 2: Medium difficulty
        m_smallFishConfig[2] = { 0.5f, 100.0f, m_windowSize.y - 100.0f, true };
        m_mediumFishConfig[2] = { 0.7f, 150.0f, m_windowSize.y - 150.0f, false }; // Keep as is
        m_largeFishConfig[2] = { 0.5f, 200.0f, m_windowSize.y - 200.0f, true };   // Keep as is

        // Level 3: Hard difficulty
        m_smallFishConfig[3] = { 1.5f, 100.0f, m_windowSize.y - 100.0f, true }; 
        m_mediumFishConfig[3] = { 0.7f, 150.0f, m_windowSize.y - 150.0f, false }; // Keep as is
        m_largeFishConfig[3] = { 0.7f, 200.0f, m_windowSize.y - 200.0f, true };   // Keep as is

        // Setup spawners
        configureSpawnersForLevel(1);
    }

    void FishSpawner::update(sf::Time deltaTime, int currentLevel)
    {
        if (currentLevel != m_currentLevel)
        {
            setLevel(currentLevel);
        }
        updateSpawners(deltaTime);
    }

    void FishSpawner::setLevel(int level)
    {
        m_currentLevel = std::max(1, level);
        configureSpawnersForLevel(m_currentLevel);
    }

    void FishSpawner::updateSpawners(sf::Time deltaTime)
    {
        // Update all spawners
        m_smallSpawner.update(deltaTime);
        m_mediumSpawner.update(deltaTime);
        m_largeSpawner.update(deltaTime);

        // Collect spawned fish
        auto smallFish = m_smallSpawner.collectSpawned();
        auto mediumFish = m_mediumSpawner.collectSpawned();
        auto largeFish = m_largeSpawner.collectSpawned();

        // Move all to main container
        std::move(smallFish.begin(), smallFish.end(), std::back_inserter(m_spawnedFish));
        std::move(mediumFish.begin(), mediumFish.end(), std::back_inserter(m_spawnedFish));
        std::move(largeFish.begin(), largeFish.end(), std::back_inserter(m_spawnedFish));
    }

    void FishSpawner::configureSpawnersForLevel(int level)
    {
        int configLevel = std::min(level, 3);

        // Configure small fish spawner
        const auto& smallConfig = m_smallFishConfig[configLevel];
        SpawnerConfig<SmallFish> smallSpawnerConfig;
        smallSpawnerConfig.spawnRate = smallConfig.spawnRate;
        smallSpawnerConfig.minBounds = sf::Vector2f(smallConfig.fromLeft ? -SPAWN_MARGIN : m_windowSize.x + SPAWN_MARGIN, smallConfig.minY);
        smallSpawnerConfig.maxBounds = sf::Vector2f(smallConfig.fromLeft ? -SPAWN_MARGIN : m_windowSize.x + SPAWN_MARGIN, smallConfig.maxY);
        smallSpawnerConfig.customizer = [this, &smallConfig](SmallFish& fish) {
            fish.setDirection(smallConfig.fromLeft ? 1.0f : -1.0f, 0.0f);
            fish.setWindowBounds(m_windowSize);
            };

        m_smallSpawner.setConfig(smallSpawnerConfig);
        m_smallSpawner.setFactory([level]() { return std::make_unique<SmallFish>(level); });

        // Configure medium fish spawner
        const auto& mediumConfig = m_mediumFishConfig[configLevel];
        SpawnerConfig<MediumFish> mediumSpawnerConfig;
        mediumSpawnerConfig.spawnRate = mediumConfig.spawnRate;
        mediumSpawnerConfig.minBounds = sf::Vector2f(mediumConfig.fromLeft ? -SPAWN_MARGIN : m_windowSize.x + SPAWN_MARGIN, mediumConfig.minY);
        mediumSpawnerConfig.maxBounds = sf::Vector2f(mediumConfig.fromLeft ? -SPAWN_MARGIN : m_windowSize.x + SPAWN_MARGIN, mediumConfig.maxY);
        mediumSpawnerConfig.customizer = [this, &mediumConfig](MediumFish& fish) {
            fish.setDirection(mediumConfig.fromLeft ? 1.0f : -1.0f, 0.0f);
            fish.setWindowBounds(m_windowSize);
            };

        m_mediumSpawner.setConfig(mediumSpawnerConfig);
        m_mediumSpawner.setFactory([level]() { return std::make_unique<MediumFish>(level); });

        // Configure large fish spawner
        const auto& largeConfig = m_largeFishConfig[configLevel];
        SpawnerConfig<LargeFish> largeSpawnerConfig;
        largeSpawnerConfig.spawnRate = largeConfig.spawnRate;
        largeSpawnerConfig.minBounds = sf::Vector2f(largeConfig.fromLeft ? -SPAWN_MARGIN : m_windowSize.x + SPAWN_MARGIN, largeConfig.minY);
        largeSpawnerConfig.maxBounds = sf::Vector2f(largeConfig.fromLeft ? -SPAWN_MARGIN : m_windowSize.x + SPAWN_MARGIN, largeConfig.maxY);
        largeSpawnerConfig.customizer = [this, &largeConfig](LargeFish& fish) {
            fish.setDirection(largeConfig.fromLeft ? 1.0f : -1.0f, 0.0f);
            fish.setWindowBounds(m_windowSize);
            };

        m_largeSpawner.setConfig(largeSpawnerConfig);
        m_largeSpawner.setFactory([level]() { return std::make_unique<LargeFish>(level); });
    }
}