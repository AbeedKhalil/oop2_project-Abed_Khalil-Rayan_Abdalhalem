#include "PowerUp.h"
#include "GameConstants.h"
#include "Utils/DrawHelpers.h"
#include <cmath>
#include <algorithm>

namespace FishGame
{
    // PowerUp base class implementation
    PowerUp::PowerUp(PowerUpType type, sf::Time duration)
        : BonusItem(BonusType::PowerUp, 0)
        , m_powerUpType(type)
        , m_duration(duration)
        , m_pulseAnimation(0.0f)
    {
        m_radius = 25.0f;
        m_lifetime = sf::seconds(15.0f);

    }

    void PowerUp::commonUpdate(sf::Time deltaTime, float pulseSpeed,
        float freqMul, float ampMul)
    {
        if (!updateLifetime(deltaTime))
            return;

        m_pulseAnimation += deltaTime.asSeconds() * pulseSpeed;
        m_position.y = m_baseY + computeBobbingOffset(freqMul, ampMul);

        if (auto sprite = getSpriteComponent())
            sprite->update(deltaTime);
    }

    // Do NOT implement update() here - let derived classes handle it

    // ScoreDoublerPowerUp implementation
    ScoreDoublerPowerUp::ScoreDoublerPowerUp()
        : PowerUp(PowerUpType::ScoreDoubler,
            sf::seconds(Constants::SCORE_DOUBLER_POWERUP_DURATION))
    {
    }

void ScoreDoublerPowerUp::update(sf::Time deltaTime)
{
    commonUpdate(deltaTime, 3.0f);
}


    // FrenzyStarterPowerUp implementation
    FrenzyStarterPowerUp::FrenzyStarterPowerUp()
        : PowerUp(PowerUpType::FrenzyStarter, sf::Time::Zero)
        , m_sparkAnimation(0.0f)
    {
    }

void FrenzyStarterPowerUp::update(sf::Time deltaTime)
{
    commonUpdate(deltaTime, 4.0f, 2.0f);
    m_sparkAnimation += deltaTime.asSeconds() * 10.0f;
}


    // PowerUpManager implementation
    PowerUpManager::PowerUpManager()
        : m_activePowerUps()
    {
        m_activePowerUps.reserve(4); // Pre-allocate for typical number of simultaneous power-ups
    }

    void PowerUpManager::activatePowerUp(PowerUpType type, sf::Time duration)
    {
        // Check if power-up already active
        auto existing = findPowerUp([type](const ActivePowerUp& p) { return p.type == type; });

        if (existing != m_activePowerUps.end())
        {
            // Extend duration
            existing->remainingTime = std::max(existing->remainingTime, duration);
        }
        else
        {
            // Add new power-up
            m_activePowerUps.push_back({ type, duration });
        }
    }

    void PowerUpManager::update(sf::Time deltaTime)
    {
        // Update all active power-ups
        std::for_each(m_activePowerUps.begin(), m_activePowerUps.end(),
            [deltaTime](ActivePowerUp& powerUp) {
                powerUp.remainingTime -= deltaTime;
            });

        // Remove expired power-ups using erase-remove idiom
        m_activePowerUps.erase(
            std::remove_if(m_activePowerUps.begin(), m_activePowerUps.end(),
                [](const ActivePowerUp& powerUp) {
                    return powerUp.remainingTime <= sf::Time::Zero;
                }),
            m_activePowerUps.end()
        );
    }

    void PowerUpManager::reset()
    {
        m_activePowerUps.clear();
    }

    bool PowerUpManager::isActive(PowerUpType type) const
    {
        return std::any_of(m_activePowerUps.begin(), m_activePowerUps.end(),
            [type](const ActivePowerUp& powerUp) {
                return powerUp.type == type;
            });
    }

    sf::Time PowerUpManager::getRemainingTime(PowerUpType type) const
    {
        auto it = std::find_if(m_activePowerUps.begin(), m_activePowerUps.end(),
            [type](const ActivePowerUp& powerUp) {
                return powerUp.type == type;
            });

        return (it != m_activePowerUps.end()) ? it->remainingTime : sf::Time::Zero;
    }

    float PowerUpManager::getScoreMultiplier() const
    {
        float multiplier = 1.0f;

        if (isActive(PowerUpType::ScoreDoubler))
        {
            multiplier *= Constants::SCORE_DOUBLER_MULTIPLIER;
        }

        // Add other score-affecting power-ups here

        return multiplier;
    }

    float PowerUpManager::getSpeedMultiplier() const
    {
        return isActive(PowerUpType::SpeedBoost) ? Constants::SPEED_BOOST_MULTIPLIER : 1.0f;
    }

    std::vector<PowerUpType> PowerUpManager::getActivePowerUps() const
    {
        std::vector<PowerUpType> activeTypes;
        activeTypes.reserve(m_activePowerUps.size());

        std::transform(m_activePowerUps.begin(), m_activePowerUps.end(),
            std::back_inserter(activeTypes),
            [](const ActivePowerUp& powerUp) { return powerUp.type; });

        return activeTypes;
    }
}