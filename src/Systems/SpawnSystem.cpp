#include "SpawnSystem.h"
#include "SpriteManager.h"
#include "GameConstants.h"

namespace FishGame
{
    SpawnSystem::SpawnSystem(SpriteManager& sprites, std::mt19937& rng, int& currentLevel, const sf::Font& font)
        : m_spriteManager(sprites)
        , m_randomEngine(rng)
        , m_currentLevel(currentLevel)
        , m_font(font)
        , m_xDist(Constants::SAFE_SPAWN_PADDING,
            Constants::WINDOW_WIDTH - Constants::SAFE_SPAWN_PADDING)
        , m_yDist(Constants::SAFE_SPAWN_PADDING,
            Constants::WINDOW_HEIGHT - Constants::SAFE_SPAWN_PADDING)
        , m_hazardTypeDist(0, 1)
        , m_powerUpTypeDist(0, 2)
    {
    }

    std::unique_ptr<Hazard> SpawnSystem::spawnRandomHazard()
    {
        std::unique_ptr<Hazard> hazard;

        switch (m_hazardTypeDist(m_randomEngine))
        {
        case 0:
            if (m_currentLevel >= 6)
            {
                hazard = std::make_unique<Bomb>();
                static_cast<Bomb*>(hazard.get())->initializeSprite(m_spriteManager);
            }
            break;
        case 1:
            if (m_currentLevel >= 4)
            {
                hazard = std::make_unique<Jellyfish>();
                static_cast<Jellyfish*>(hazard.get())->initializeSprite(m_spriteManager);
                hazard->setVelocity(0.0f, 20.0f);
            }
            break;
        }

        if (hazard)
        {
            hazard->setPosition(generateRandomPosition());
        }

        return hazard;
    }

    std::unique_ptr<PowerUp> SpawnSystem::spawnRandomPowerUp()
    {
        std::unique_ptr<PowerUp> powerUp;

        int type = m_powerUpTypeDist(m_randomEngine);
        if (m_currentLevel < 2 && (type == 0 || type == 2))
            type = 1;

        switch (type)
        {
        case 0:
            powerUp = std::make_unique<FreezePowerUp>();
            if (auto* freeze = dynamic_cast<FreezePowerUp*>(powerUp.get()))
                freeze->setFont(m_font);
            break;
        case 1:
            powerUp = std::make_unique<ExtraLifePowerUp>();
            if (auto* life = dynamic_cast<ExtraLifePowerUp*>(powerUp.get()))
                life->initializeSprite(m_spriteManager);
            break;
        case 2:
            powerUp = std::make_unique<SpeedBoostPowerUp>();
            if (auto* speed = dynamic_cast<SpeedBoostPowerUp*>(powerUp.get()))
                speed->initializeSprite(m_spriteManager);
            break;
        }

        if (powerUp)
        {
            sf::Vector2f pos = generateRandomPosition();
            powerUp->setPosition(pos);
            powerUp->m_baseY = pos.y;
        }

        return powerUp;
    }

    sf::Vector2f SpawnSystem::generateRandomPosition()
    {
        return sf::Vector2f(m_xDist(m_randomEngine), m_yDist(m_randomEngine));
    }
}

