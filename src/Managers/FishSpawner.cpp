#include "FishSpawner.h"
#include "GameConstants.h"
#include "SpriteManager.h"
#include "Strategy.h"
#include <algorithm>
#include <array>

namespace FishGame
{
    using namespace Constants;

    FishSpawner::FishSpawner(const sf::Vector2u& windowSize, SpriteManager& spriteManager)
        : m_windowSize(windowSize)
        , m_spawnedFish()
        , m_smallSpawner()
        , m_mediumSpawner()
        , m_largeSpawner()
        , m_currentLevel(1)
        , m_randomEngine(std::random_device{}())
        , m_spriteManager(&spriteManager)
    {
        // Initialize spawn configurations for each level
        // Emphasize medium fish by reducing small fish spawn rates

        // Level 1: Easier difficulty
        // Fewer small fish and more medium fish
        m_smallFishConfig[1] = { 0.04f, 100.0f, m_windowSize.y - 100.0f};
        m_mediumFishConfig[1] = { 1.2f, 150.0f, m_windowSize.y - 150.0f};
        m_largeFishConfig[1] = { 0.3f, 200.0f, m_windowSize.y - 200.0f};    // Reduced from 0.8f

        // Level 2: Medium difficulty
        // Reduce small fish frequency and boost medium fish
        m_smallFishConfig[2] = { 0.1f, 100.0f, m_windowSize.y - 100.0f};
        m_mediumFishConfig[2] = { 1.4f, 150.0f, m_windowSize.y - 150.0f};
        m_largeFishConfig[2] = { 0.3f, 200.0f, m_windowSize.y - 200.0f};    // Reduced from 0.5f

        // Level 3: Hard difficulty
        // Keep small fish scarce and further increase medium fish
        m_smallFishConfig[3] = { 0.15f, 100.0f, m_windowSize.y - 100.0f};
        m_mediumFishConfig[3] = { 1.6f, 150.0f, m_windowSize.y - 150.0f};
        m_largeFishConfig[3] = { 0.4f, 200.0f, m_windowSize.y - 200.0f};    // Reduced from 0.7f

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
        // Allow spawning from either side by using the full horizontal range
        smallSpawnerConfig.minBounds = sf::Vector2f(-SPAWN_MARGIN, smallConfig.minY);
        smallSpawnerConfig.maxBounds = sf::Vector2f(m_windowSize.x + SPAWN_MARGIN, smallConfig.maxY);
        smallSpawnerConfig.customizer = [this](SmallFish& fish) {
            bool fromLeft = m_randomEngine() % 2 == 0;
            float x = fromLeft ? -SPAWN_MARGIN : m_windowSize.x + SPAWN_MARGIN;
            fish.setPosition(x, fish.getPosition().y);
            fish.setDirection(fromLeft ? 1.0f : -1.0f, 0.0f);
            fish.setWindowBounds(m_windowSize);
            fish.initializeSprite(*m_spriteManager);
            fish.setMovementStrategy(std::make_unique<RandomWanderStrategy>());
            };

        m_smallSpawner.setConfig(smallSpawnerConfig);
        m_smallSpawner.setFactory([level]() { return std::make_unique<SmallFish>(level); });

        // Configure medium fish spawner
        const auto& mediumConfig = m_mediumFishConfig[configLevel];
        SpawnerConfig<MediumFish> mediumSpawnerConfig;
        mediumSpawnerConfig.spawnRate = mediumConfig.spawnRate;
        mediumSpawnerConfig.minBounds = sf::Vector2f(-SPAWN_MARGIN, mediumConfig.minY);
        mediumSpawnerConfig.maxBounds = sf::Vector2f(m_windowSize.x + SPAWN_MARGIN, mediumConfig.maxY);
        mediumSpawnerConfig.customizer = [this](MediumFish& fish) {
            bool fromLeft = m_randomEngine() % 2 == 0;
            float x = fromLeft ? -SPAWN_MARGIN : m_windowSize.x + SPAWN_MARGIN;
            fish.setPosition(x, fish.getPosition().y);
            fish.setDirection(fromLeft ? 1.0f : -1.0f, 0.0f);
            fish.setWindowBounds(m_windowSize);
            fish.initializeSprite(*m_spriteManager);

            static const std::array<sf::Color, 3> colors = {
                sf::Color(255, 255, 255, 255),           // Original (no tint)
                sf::Color(255, 180, 180, 255),          // Warm red
				sf::Color(255 , 150 , 160, 255)        // Salmon Pink
            };
            std::uniform_int_distribution<int> dist(0, colors.size() - 1);
            fish.setBaseColor(colors[dist(m_randomEngine)]);
            fish.setMovementStrategy(std::make_unique<RandomWanderStrategy>());
            };

        m_mediumSpawner.setConfig(mediumSpawnerConfig);
        m_mediumSpawner.setFactory([level]() { return std::make_unique<MediumFish>(level); });

        // Configure large fish spawner
        const auto& largeConfig = m_largeFishConfig[configLevel];
        SpawnerConfig<LargeFish> largeSpawnerConfig;
        largeSpawnerConfig.spawnRate = largeConfig.spawnRate;
        largeSpawnerConfig.minBounds = sf::Vector2f(-SPAWN_MARGIN, largeConfig.minY);
        largeSpawnerConfig.maxBounds = sf::Vector2f(m_windowSize.x + SPAWN_MARGIN, largeConfig.maxY);
        largeSpawnerConfig.customizer = [this](LargeFish& fish) {
            bool fromLeft = m_randomEngine() % 2 == 0;
            float x = fromLeft ? -SPAWN_MARGIN : m_windowSize.x + SPAWN_MARGIN;
            fish.setPosition(x, fish.getPosition().y);
            fish.setDirection(fromLeft ? 1.0f : -1.0f, 0.0f);
            fish.setWindowBounds(m_windowSize);
            fish.initializeSprite(*m_spriteManager);
            fish.setMovementStrategy(std::make_unique<AggressiveChaseStrategy>(nullptr));
            };

        m_largeSpawner.setConfig(largeSpawnerConfig);
        m_largeSpawner.setFactory([level]() { return std::make_unique<LargeFish>(level); });
    }
}
