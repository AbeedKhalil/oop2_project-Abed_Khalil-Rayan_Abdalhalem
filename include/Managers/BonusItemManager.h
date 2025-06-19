#pragma once

#include "BonusItem.h"
#include "PowerUp.h"
#include <vector>
#include <algorithm>
#include <functional>

namespace FishGame
{
    class SpriteManager;
    // Enhanced spawner that can handle font requirements
    template<typename T>
    class EnhancedBonusSpawner
    {
        static_assert(std::is_base_of_v<BonusItem, T>, "T must be derived from BonusItem");

    public:
        EnhancedBonusSpawner(float spawnRate, const sf::Vector2u& windowSize, const sf::Font* font = nullptr)
            : m_spawnRate(spawnRate)
            , m_spawnTimer(sf::Time::Zero)
            , m_windowSize(windowSize)
            , m_font(font)
            , m_randomEngine(std::random_device{}())
            , m_xDistribution(0.0f, 1.0f)  // Initialize with valid range
            , m_yDistribution(0.0f, 1.0f)  // Initialize with valid range
            , m_enabled(true)
        {
            // Set up distributions with validation
            float margin = 100.0f;
            float maxX = static_cast<float>(windowSize.x) - margin;
            float maxY = static_cast<float>(windowSize.y) - margin;

            // Ensure valid ranges
            if (maxX > margin)
            {
                m_xDistribution = std::uniform_real_distribution<float>(margin, maxX);
            }
            else
            {
                // Fallback for small windows
                float safeMargin = 10.0f;
                m_xDistribution = std::uniform_real_distribution<float>(safeMargin,
                    std::max(static_cast<float>(windowSize.x) - safeMargin, safeMargin + 1.0f));
            }

            if (maxY > margin)
            {
                m_yDistribution = std::uniform_real_distribution<float>(margin, maxY);
            }
            else
            {
                // Fallback for small windows
                float safeMargin = 10.0f;
                m_yDistribution = std::uniform_real_distribution<float>(safeMargin,
                    std::max(static_cast<float>(windowSize.y) - safeMargin, safeMargin + 1.0f));
            }
        }

        void update(sf::Time deltaTime)
        {
            if (!m_enabled)
                return;

            m_spawnTimer += deltaTime;

            if (m_spawnTimer.asSeconds() >= 1.0f / m_spawnRate)
            {
                m_spawnTimer = sf::Time::Zero;
                m_shouldSpawn = true;
            }
        }

        std::unique_ptr<BonusItem> spawn()
        {
            if (!m_shouldSpawn)
                return nullptr;

            m_shouldSpawn = false;
            auto item = createItem();

            if (item)
            {
                float x = m_xDistribution(m_randomEngine);
                float y = m_yDistribution(m_randomEngine);

                // Special positioning for oysters - place at bottom
                if (dynamic_cast<PearlOyster*>(item.get()))
                {
                    y = static_cast<float>(m_windowSize.y) - 50.0f; // Near bottom
                }

                item->setPosition(x, y);

                // Store base Y position for bobbing animation
                if (BonusItem* bonusItem = dynamic_cast<BonusItem*>(item.get()))
                {
                    bonusItem->m_baseY = y;
                }
            }

            return item;
        }

        void setSpawnRate(float rate) { m_spawnRate = rate; }
        void setEnabled(bool enabled) { m_enabled = enabled; }

    private:
        std::unique_ptr<BonusItem> createItem()
        {
            auto item = std::make_unique<T>();

            // Set font if needed for text-based items
            if constexpr (std::is_same_v<T, ScoreDoublerPowerUp>)
            {
                if (m_font)
                {
                    static_cast<ScoreDoublerPowerUp*>(item.get())->setFont(*m_font);
                }
            }

            return item;
        }

    private:
        float m_spawnRate;
        sf::Time m_spawnTimer;
        bool m_shouldSpawn = false;
        bool m_enabled;
        sf::Vector2u m_windowSize;
        const sf::Font* m_font;

        std::mt19937 m_randomEngine;
        std::uniform_real_distribution<float> m_xDistribution;
        std::uniform_real_distribution<float> m_yDistribution;
    };

    // Bonus Item Manager - handles all bonus item spawning and management
    class BonusItemManager
    {
    public:
        BonusItemManager(const sf::Vector2u& windowSize, const sf::Font& font,
            SpriteManager& spriteManager);
        ~BonusItemManager() = default;

        // Delete copy operations
        BonusItemManager(const BonusItemManager&) = delete;
        BonusItemManager& operator=(const BonusItemManager&) = delete;

        // Allow move operations
        BonusItemManager(BonusItemManager&&) = default;
        BonusItemManager& operator=(BonusItemManager&&) = default;

        // Update all spawners
        void update(sf::Time deltaTime);

        // Get newly spawned items
        std::vector<std::unique_ptr<BonusItem>> collectSpawnedItems();

        // Level-based configuration
        void setLevel(int level);

        // Enable/disable specific spawners
        void setStarfishEnabled(bool enabled);
        void setOysterEnabled(bool enabled);
        void setPowerUpsEnabled(bool enabled);

        // Force spawn specific items
        void spawnStarfish();
        void spawnOyster();
        void spawnRandomPowerUp();

    private:
        void updatePowerUpSpawning(sf::Time deltaTime);
        std::unique_ptr<PowerUp> createRandomPowerUp();

        template<typename T, typename Init = std::function<void(T&)>>
        void spawnItem(Init init = {});

        template<typename T>
        void addItem(std::unique_ptr<T> item);

    private:
        const sf::Font& m_font;
        sf::Vector2u m_windowSize;
        SpriteManager* m_spriteManager;
        int m_currentLevel;

        // Spawners for different item types
        std::unique_ptr<EnhancedBonusSpawner<Starfish>> m_starfishSpawner;
        std::unique_ptr<EnhancedBonusSpawner<PearlOyster>> m_oysterSpawner;

        // Power-up spawning
        sf::Time m_powerUpSpawnTimer;
        float m_powerUpSpawnInterval;
        bool m_powerUpsEnabled;

        // Spawned items buffer
        std::vector<std::unique_ptr<BonusItem>> m_spawnedItems;

        // Random number generation for power-ups
        std::mt19937 m_randomEngine;
        std::uniform_real_distribution<float> m_positionDist;

        // Base spawn rates
        static constexpr float m_baseStarfishRate = 0.2f;
        static constexpr float m_baseOysterRate = 0.1f;
    static constexpr float m_basePowerUpInterval = 20.0f;
    };

    template<typename T, typename Init>
    void BonusItemManager::spawnItem(Init init)
    {
        auto item = std::make_unique<T>();
        if (init)
            init(*item);
        addItem(std::move(item));
    }

    template<typename T>
    void BonusItemManager::addItem(std::unique_ptr<T> item)
    {
        if (!item)
            return;
        float x = m_positionDist(m_randomEngine);
        float y = m_positionDist(m_randomEngine);
        item->setPosition(x, y);
        m_spawnedItems.push_back(std::move(item));
    }
}