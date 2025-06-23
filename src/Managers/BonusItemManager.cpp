#include "BonusItemManager.h"
#include "SpriteManager.h"
#include <algorithm>
#include <numeric>
#include <array>
#include <ExtendedPowerUps.h>
#include "PowerUpFactory.h"

namespace FishGame
{
    BonusItemManager::BonusItemManager(const sf::Vector2u& windowSize, const sf::Font& font,
        SpriteManager& spriteManager)
        : m_font(font)
        , m_windowSize(windowSize)
        , m_spriteManager(&spriteManager)
        , m_currentLevel(1)
        , m_starfishSpawner(std::make_unique<EnhancedBonusSpawner<Starfish>>(m_baseStarfishRate, windowSize))
        , m_powerUpSpawnTimer(sf::Time::Zero)
        , m_powerUpSpawnInterval(m_basePowerUpInterval)
        , m_powerUpsEnabled(true)
        , m_spawnedItems()
        , m_randomEngine(std::random_device{}())
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

        // Spawn starfish
        if (auto starfish = m_starfishSpawner->spawn())
        {
            if (m_spriteManager)
                static_cast<Starfish*>(starfish.get())->initializeSprite(*m_spriteManager);
            m_spawnedItems.push_back(std::move(starfish));
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

        // Decrease power-up spawn interval for higher levels
        m_powerUpSpawnInterval = m_basePowerUpInterval / (1.0f + (m_currentLevel - 1) * 0.1f);
    }

    void BonusItemManager::setStarfishEnabled(bool enabled)
    {
        m_starfishSpawner->setEnabled(enabled);
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

    void BonusItemManager::spawnRandomPowerUp()
    {
        if (auto powerUp = createRandomPowerUp())
        {
            float x = m_positionDist(m_randomEngine);
            float y = m_positionDist(m_randomEngine);
            powerUp->setPosition(x, y);

            powerUp->m_baseY = y;

            if (m_spriteManager)
            {
                if (auto* life = dynamic_cast<ExtraLifePowerUp*>(powerUp.get()))
                    life->initializeSprite(*m_spriteManager);
                else if (auto* speed = dynamic_cast<SpeedBoostPowerUp*>(powerUp.get()))
                    speed->initializeSprite(*m_spriteManager);
            }

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
        std::vector<PowerUpType> types;
        if (m_currentLevel >= 2)
            types = { PowerUpType::ScoreDoubler, PowerUpType::FrenzyStarter,
                PowerUpType::ScoreDoubler, PowerUpType::FrenzyStarter,
                PowerUpType::Freeze, PowerUpType::ExtraLife, PowerUpType::SpeedBoost };
        else
            types = { PowerUpType::ScoreDoubler, PowerUpType::FrenzyStarter,
                PowerUpType::ScoreDoubler, PowerUpType::FrenzyStarter,
                PowerUpType::ScoreDoubler, PowerUpType::FrenzyStarter, PowerUpType::ExtraLife };

        int index = std::uniform_int_distribution<int>(0, static_cast<int>(types.size()) - 1)(m_randomEngine);
        PowerUpType type = types[index];

        using CreateFunc = std::unique_ptr<PowerUp>(*)(const sf::Font*);
        static constexpr std::array<CreateFunc, 6> creators = {
            &PowerUpFactory<PowerUpType::ScoreDoubler>::create,
            &PowerUpFactory<PowerUpType::FrenzyStarter>::create,
            &PowerUpFactory<PowerUpType::SpeedBoost>::create,
            &PowerUpFactory<PowerUpType::Freeze>::create,
            &PowerUpFactory<PowerUpType::ExtraLife>::create,
            &PowerUpFactory<PowerUpType::AddTime>::create
        };

        return creators[static_cast<size_t>(type)](&m_font);
    }
}
