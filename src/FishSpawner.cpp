// FishSpawner.cpp
#include "FishSpawner.h"
#include "GameConstants.h"
#include <algorithm>
#include <numeric>

namespace FishGame
{
    using namespace Constants;

    FishSpawner::FishSpawner(const sf::Vector2u& windowSize)
        : m_windowSize(windowSize)
        , m_spawnedFish()
        , m_randomEngine(std::random_device{}())
        , m_yDistribution(100.0f, windowSize.y - 100.0f)
        , m_spawnChance(0.0f, 1.0f)
        , m_smallFishTimer(sf::Time::Zero)
        , m_mediumFishTimer(sf::Time::Zero)
        , m_largeFishTimer(sf::Time::Zero)
        , m_currentLevel(1)
    {
        // Initialize spawn configurations for each level
        // Level 1: All fish types with lower spawn rates
        m_smallFishConfig[1] = { 1.5f, 100.0f, m_windowSize.y - 100.0f, true };
        m_mediumFishConfig[1] = { 0.8f, 150.0f, m_windowSize.y - 150.0f, false };
        m_largeFishConfig[1] = { 0.4f, 200.0f, m_windowSize.y - 200.0f, true };

        // Level 2: Increased spawn rates
        m_smallFishConfig[2] = { 2.0f, 100.0f, m_windowSize.y - 100.0f, true };
        m_mediumFishConfig[2] = { 1.2f, 150.0f, m_windowSize.y - 150.0f, false };
        m_largeFishConfig[2] = { 0.6f, 200.0f, m_windowSize.y - 200.0f, true };

        // Level 3+: Higher spawn rates
        m_smallFishConfig[3] = { 2.5f, 100.0f, m_windowSize.y - 100.0f, true };
        m_mediumFishConfig[3] = { 1.5f, 150.0f, m_windowSize.y - 150.0f, false };
        m_largeFishConfig[3] = { 0.8f, 200.0f, m_windowSize.y - 200.0f, true };
    }

    void FishSpawner::update(sf::Time deltaTime, int currentLevel)
    {
        m_currentLevel = currentLevel;
        updateSpawnTimers(deltaTime);
    }

    void FishSpawner::setLevel(int level)
    {
        m_currentLevel = std::max(1, level);
    }

    template<typename FishType>
    void FishSpawner::spawnFish(bool fromLeft)
    {
        auto fish = FishFactory<FishType>::create(m_currentLevel);

        // Set random Y position
        float yPos = m_yDistribution(m_randomEngine);

        // Set starting position based on direction
        float xPos = fromLeft ? -SPAWN_MARGIN : m_windowSize.x + SPAWN_MARGIN;
        fish->setPosition(xPos, yPos);

        // Set direction
        float dirX = fromLeft ? 1.0f : -1.0f;
        fish->setDirection(dirX, 0.0f);

        // Set window bounds
        fish->setWindowBounds(m_windowSize);

        m_spawnedFish.push_back(std::move(fish));
    }

    void FishSpawner::updateSpawnTimers(sf::Time deltaTime)
    {
        // Get current level configs (use level 3 config for levels > 3)
        int configLevel = std::min(m_currentLevel, 3);

        const auto& smallConfig = m_smallFishConfig[configLevel];
        const auto& mediumConfig = m_mediumFishConfig[configLevel];
        const auto& largeConfig = m_largeFishConfig[configLevel];

        // Update spawn timers and spawn fish accordingly
        if (shouldSpawn(m_smallFishTimer, smallConfig.spawnRate, deltaTime))
        {
            spawnFish<SmallFish>(smallConfig.fromLeft);
            m_smallFishTimer = sf::Time::Zero;
        }

        if (shouldSpawn(m_mediumFishTimer, mediumConfig.spawnRate, deltaTime))
        {
            spawnFish<MediumFish>(mediumConfig.fromLeft);
            m_mediumFishTimer = sf::Time::Zero;
        }

        if (shouldSpawn(m_largeFishTimer, largeConfig.spawnRate, deltaTime))
        {
            spawnFish<LargeFish>(largeConfig.fromLeft);
            m_largeFishTimer = sf::Time::Zero;
        }
    }

    bool FishSpawner::shouldSpawn(sf::Time& timer, float spawnRate, sf::Time deltaTime)
    {
        if (spawnRate <= 0.0f)
            return false;

        timer += deltaTime;
        sf::Time spawnInterval = sf::seconds(1.0f / spawnRate);

        return timer >= spawnInterval;
    }

    // Explicit template instantiations
    template void FishSpawner::spawnFish<SmallFish>(bool);
    template void FishSpawner::spawnFish<MediumFish>(bool);
    template void FishSpawner::spawnFish<LargeFish>(bool);
}