#include "EnhancedFishSpawner.h"
#include "FishFactory.h"
#include <algorithm>

namespace FishGame
{
    EnhancedFishSpawner::EnhancedFishSpawner(const sf::Vector2u& windowSize)
        : FishSpawner(windowSize)
        , m_barracudaSpawner()
        , m_pufferfishSpawner()
        , m_angelfishSpawner()
        , m_specialConfig()
        , m_schoolingSystem(nullptr)
        , m_schoolChanceDist(0.0f, 1.0f)
        , m_schoolSizeDist(3, 6)
    {
        // Initialize special spawn timers
        m_specialSpawnTimers["barracuda"] = sf::Time::Zero;
        m_specialSpawnTimers["pufferfish"] = sf::Time::Zero;
        m_specialSpawnTimers["angelfish"] = sf::Time::Zero;

        // Configure special spawners
        configureSpecialSpawners(1);
    }

    void EnhancedFishSpawner::update(sf::Time deltaTime, int currentLevel)
    {
        // Update base spawners
        FishSpawner::update(deltaTime, currentLevel);

        // Update special fish spawning
        if (currentLevel >= 2)  // Special fish start appearing from level 2
        {
            // Update spawn timers
            std::for_each(m_specialSpawnTimers.begin(), m_specialSpawnTimers.end(),
                [deltaTime](auto& pair)
                {
                    pair.second += deltaTime;
                });

            // Spawn special fish types
            spawnSpecialFish<Barracuda>(m_specialConfig.barracudaSpawnRate, deltaTime);
            spawnSpecialFish<Pufferfish>(m_specialConfig.pufferfishSpawnRate, deltaTime);
            spawnSpecialFish<Angelfish>(m_specialConfig.angelfishSpawnRate, deltaTime);
        }

        // Check for school spawning
        if (m_schoolingSystem && m_schoolChanceDist(m_randomEngine) < m_specialConfig.schoolSpawnChance)
        {
            size_t schoolSize = m_schoolSizeDist(m_randomEngine);

            // Randomly choose fish type for school
            if (m_randomEngine() % 2 == 0)
            {
                spawnSchool<SmallFish>(schoolSize);
            }
            else
            {
                spawnSchool<MediumFish>(schoolSize - 1);  // Smaller schools for medium fish
            }
        }
    }

    template<typename SpecialFishType>
    void EnhancedFishSpawner::spawnSpecialFish(float spawnRate, sf::Time deltaTime)
    {
        std::string typeName;

        // Determine type name for timer lookup
        if constexpr (std::is_same_v<SpecialFishType, Barracuda>)
        {
            typeName = "barracuda";
        }
        else if constexpr (std::is_same_v<SpecialFishType, Pufferfish>)
        {
            typeName = "pufferfish";
        }
        else if constexpr (std::is_same_v<SpecialFishType, Angelfish>)
        {
            typeName = "angelfish";
        }

        // Check spawn timer
        if (m_specialSpawnTimers[typeName].asSeconds() >= 1.0f / spawnRate)
        {
            m_specialSpawnTimers[typeName] = sf::Time::Zero;

            // Use factory to create special fish
            auto fish = FishFactory<SpecialFishType>::create(m_currentLevel);

            // Determine spawn position
            bool fromLeft = m_randomEngine() % 2 == 0;
            float y = std::uniform_real_distribution<float>(100.0f, m_windowSize.y - 100.0f)(m_randomEngine);
            float x = fromLeft ? -50.0f : m_windowSize.x + 50.0f;

            fish->setPosition(x, y);
            fish->setDirection(fromLeft ? 1.0f : -1.0f, 0.0f);
            fish->setWindowBounds(m_windowSize);

            // Add to spawned fish
            m_spawnedFish.push_back(std::move(fish));
        }
    }

    template<typename FishType>
    void EnhancedFishSpawner::spawnSchool(size_t count)
    {
        if (!m_schoolingSystem)
            return;

        // Create a new school
        SchoolConfig config;
        config.maxMembers = count + 2;  // Allow some growth

        if constexpr (std::is_same_v<FishType, SmallFish>)
        {
            config.fishSize = FishSize::Small;
        }
        else if constexpr (std::is_same_v<FishType, MediumFish>)
        {
            config.fishSize = FishSize::Medium;
        }

        int schoolId = m_schoolingSystem->createSchool<FishType>(config);

        // Spawn position for the school
        bool fromLeft = m_randomEngine() % 2 == 0;
        float baseY = std::uniform_real_distribution<float>(150.0f, m_windowSize.y - 150.0f)(m_randomEngine);
        float baseX = fromLeft ? -50.0f : m_windowSize.x + 50.0f;

        // Create formation configuration
        AdvancedSpawnConfig<FishType> spawnConfig;
        spawnConfig.pattern = SpawnPattern::WaveFormation;
        spawnConfig.count = count;
        spawnConfig.spacing = 40.0f;

        // Determine movement pattern based on fish type
        if constexpr (std::is_base_of_v<AdvancedFish, FishType>)
        {
            spawnConfig.movementPattern = MovementPattern::Sinusoidal;
        }

        // Create fish formation using factory
        auto formation = ConfiguredFishFactory<FishType>::createFormation(
            spawnConfig, sf::Vector2f(baseX, baseY), m_currentLevel);

        // Add each fish to the school
        for (auto& fish : formation)
        {
            // Create school member using factory
            auto member = SchoolingFishFactory<FishType>::createFromFish(*fish, m_currentLevel);
            member->setDirection(fromLeft ? 1.0f : -1.0f, 0.0f);

            // Try to add to schooling system
            if (!m_schoolingSystem->tryAddToSchool(std::move(member)))
            {
                // If school is full, add as regular fish
                m_spawnedFish.push_back(std::move(fish));
            }
        }
    }

    void EnhancedFishSpawner::configureSpecialSpawners(int level)
    {
        // Configure barracuda spawner
        SpawnerConfig<Barracuda> barracudaConfig;
        barracudaConfig.spawnRate = m_specialConfig.barracudaSpawnRate * (1.0f + (level - 1) * 0.1f);
        barracudaConfig.minBounds = sf::Vector2f(-50.0f, 100.0f);
        barracudaConfig.maxBounds = sf::Vector2f(m_windowSize.x + 50.0f, m_windowSize.y - 100.0f);

        m_barracudaSpawner.setConfig(barracudaConfig);
        m_barracudaSpawner.setFactory([level]() { return FishFactory<Barracuda>::create(level); });

        // Configure pufferfish spawner
        SpawnerConfig<Pufferfish> pufferfishConfig;
        pufferfishConfig.spawnRate = m_specialConfig.pufferfishSpawnRate * (1.0f + (level - 1) * 0.15f);
        pufferfishConfig.minBounds = sf::Vector2f(-50.0f, 150.0f);
        pufferfishConfig.maxBounds = sf::Vector2f(m_windowSize.x + 50.0f, m_windowSize.y - 150.0f);

        m_pufferfishSpawner.setConfig(pufferfishConfig);
        m_pufferfishSpawner.setFactory([level]() { return FishFactory<Pufferfish>::create(level); });

        // Configure angelfish spawner
        SpawnerConfig<Angelfish> angelfishConfig;
        angelfishConfig.spawnRate = m_specialConfig.angelfishSpawnRate * (1.0f + (level - 1) * 0.2f);
        angelfishConfig.minBounds = sf::Vector2f(-50.0f, 50.0f);
        angelfishConfig.maxBounds = sf::Vector2f(m_windowSize.x + 50.0f, m_windowSize.y - 50.0f);

        m_angelfishSpawner.setConfig(angelfishConfig);
        m_angelfishSpawner.setFactory([level]() { return FishFactory<Angelfish>::create(level); });
    }

    // Explicit template instantiations
    template void EnhancedFishSpawner::spawnSpecialFish<Barracuda>(float, sf::Time);
    template void EnhancedFishSpawner::spawnSpecialFish<Pufferfish>(float, sf::Time);
    template void EnhancedFishSpawner::spawnSpecialFish<Angelfish>(float, sf::Time);
    template void EnhancedFishSpawner::spawnSchool<SmallFish>(size_t);
    template void EnhancedFishSpawner::spawnSchool<MediumFish>(size_t);
}