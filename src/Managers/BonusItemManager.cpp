#include "BonusItemManager.h"
#include "SpriteManager.h"
#include <algorithm>
#include <numeric>
#include <ExtendedPowerUps.h>

namespace FishGame
{
    BonusItemManager::BonusItemManager(const sf::Vector2u& windowSize, const sf::Font& font,
        SpriteManager& spriteManager)
        : m_font(font)
        , m_windowSize(windowSize)
        , m_spriteManager(&spriteManager)
        , m_currentLevel(1)
        , m_starfishSpawner(std::make_unique<EnhancedBonusSpawner<Starfish>>(m_baseStarfishRate, windowSize))
        , m_oysterSpawner(std::make_unique<EnhancedBonusSpawner<PearlOyster>>(m_baseOysterRate, windowSize))
        , m_powerUpSpawnTimer(sf::Time::Zero)
        , m_powerUpSpawnInterval(m_basePowerUpInterval)
        , m_powerUpsEnabled(true)
        , m_spawnedItems()
        , m_randomEngine(std::random_device{}())
        , m_powerUpTypeDist(0, 1)  // 2 types of power-ups for now
        , m_positionDist(0.0f, 1.0f)  // Initialize with valid range
    {
        m_spawnedItems.reserve(10);

        // Update position distribution based on window size with validation
        float minPos = 100.0f;
        float maxX = static_cast<float>(windowSize.x) - 100.0f;
        float maxY = static_cast<float>(windowSize.y) - 100.0f;

        // Ensure valid range (min < max)
        if (maxX > minPos && maxY > minPos)
        {
            m_positionDist = std::uniform_real_distribution<float>(minPos, std::min(maxX, maxY));
        }
        else
        {
            // Fallback: use entire window with small margin
            float margin = 10.0f;
            float safeMax = std::max(static_cast<float>(std::min(windowSize.x, windowSize.y)) - margin, margin + 1.0f);
            m_positionDist = std::uniform_real_distribution<float>(margin, safeMax);
        }
    }

    void BonusItemManager::update(sf::Time deltaTime)
    {
        // Update spawners
        m_starfishSpawner->update(deltaTime);
        m_oysterSpawner->update(deltaTime);

        // Spawn starfish
        if (auto starfish = m_starfishSpawner->spawn())
        {
            if (m_spriteManager)
                static_cast<Starfish*>(starfish.get())->initializeSprite(*m_spriteManager);
            m_spawnedItems.push_back(std::move(starfish));
        }

        // Spawn oysters
        if (auto oyster = m_oysterSpawner->spawn())
        {
            m_spawnedItems.push_back(std::move(oyster));
        }

        // Update power-up spawning
        if (m_powerUpsEnabled)
        {
            updatePowerUpSpawning(deltaTime);
        }
    }

    std::vector<std::unique_ptr<BonusItem>> BonusItemManager::collectSpawnedItems()
    {
        std::vector<std::unique_ptr<BonusItem>> items;
        items.swap(m_spawnedItems);
        m_spawnedItems.clear();
        return items;
    }

    void BonusItemManager::setLevel(int level)
    {
        m_currentLevel = std::max(1, level);

        // Increase spawn rates based on level
        float difficultyMultiplier = 1.0f + (m_currentLevel - 1) * 0.2f;

        m_starfishSpawner->setSpawnRate(m_baseStarfishRate * difficultyMultiplier);
        m_oysterSpawner->setSpawnRate(m_baseOysterRate * difficultyMultiplier);

        // Decrease power-up spawn interval for higher levels
        m_powerUpSpawnInterval = m_basePowerUpInterval / (1.0f + (m_currentLevel - 1) * 0.1f);
    }

    void BonusItemManager::setStarfishEnabled(bool enabled)
    {
        m_starfishSpawner->setEnabled(enabled);
    }

    void BonusItemManager::setOysterEnabled(bool enabled)
    {
        m_oysterSpawner->setEnabled(enabled);
    }

    void BonusItemManager::setPowerUpsEnabled(bool enabled)
    {
        m_powerUpsEnabled = enabled;
    }

    void BonusItemManager::spawnStarfish()
    {
        auto starfish = std::make_unique<Starfish>();

        float x = m_positionDist(m_randomEngine);
        float y = m_positionDist(m_randomEngine);
        starfish->setPosition(x, y);
        if (m_spriteManager)
            starfish->initializeSprite(*m_spriteManager);

        m_spawnedItems.push_back(std::move(starfish));
    }

    void BonusItemManager::spawnOyster()
    {
        auto oyster = std::make_unique<PearlOyster>();

        float x = m_positionDist(m_randomEngine);
        float y = m_positionDist(m_randomEngine);
        oyster->setPosition(x, y);

        m_spawnedItems.push_back(std::move(oyster));
    }

    void BonusItemManager::spawnRandomPowerUp()
    {
        if (auto powerUp = createRandomPowerUp())
        {
            float x = m_positionDist(m_randomEngine);
            float y = m_positionDist(m_randomEngine);
            powerUp->setPosition(x, y);

            m_spawnedItems.push_back(std::move(powerUp));
        }
    }

    void BonusItemManager::updatePowerUpSpawning(sf::Time deltaTime)
    {
        m_powerUpSpawnTimer += deltaTime;

        if (m_powerUpSpawnTimer.asSeconds() >= m_powerUpSpawnInterval)
        {
            m_powerUpSpawnTimer = sf::Time::Zero;
            spawnRandomPowerUp();
        }
    }

    std::unique_ptr<PowerUp> BonusItemManager::createRandomPowerUp()
    {
        // Extended to include new power-ups
        int type = std::uniform_int_distribution<int>(0, 6)(m_randomEngine);

        switch (type)
        {
        case 0:
        {
            auto powerUp = std::make_unique<ScoreDoublerPowerUp>();
            powerUp->setFont(m_font);
            return powerUp;
        }
        case 1:
            return std::make_unique<FrenzyStarterPowerUp>();

        case 2:
        {
            auto powerUp = std::make_unique<FreezePowerUp>();
            powerUp->setFont(m_font);
            return powerUp;
        }
        case 3:
            return std::make_unique<ExtraLifePowerUp>();

        case 4:
            return std::make_unique<SpeedBoostPowerUp>();

        case 5:
            // Shield power-up (you'd need to create this)
            return std::make_unique<ScoreDoublerPowerUp>(); // Placeholder

        case 6:
            // Invincibility power-up
            return std::make_unique<ScoreDoublerPowerUp>(); // Placeholder

        default:
            return nullptr;
        }
    }
}