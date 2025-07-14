#pragma once

#include <SFML/System/Time.hpp>
#include <vector>

namespace FishGame
{
    // Forward declaration to avoid circular dependency
    enum class PowerUpType;
    class IPowerUpManager
    {
    public:
        virtual ~IPowerUpManager() = default;

        virtual void activatePowerUp(PowerUpType type, sf::Time duration) = 0;
        virtual void update(sf::Time deltaTime) = 0;
        virtual void reset() = 0;
        virtual bool isActive(PowerUpType type) const = 0;
        virtual sf::Time getRemainingTime(PowerUpType type) const = 0;
        virtual float getScoreMultiplier() const = 0;
        virtual std::vector<PowerUpType> getActivePowerUps() const = 0;
        virtual float getSpeedMultiplier() const = 0;
    };
}
