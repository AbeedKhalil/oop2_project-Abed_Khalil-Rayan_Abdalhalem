#include "ExtendedPowerUps.h"
#include "Utils/DrawHelpers.h"
#include <algorithm>
#include <cmath>
#include <iterator>

namespace FishGame
{
    // FreezePowerUp implementation
    FreezePowerUp::FreezePowerUp()
        : PowerUp(PowerUpType::Freeze, sf::seconds(m_freezeDuration))
    {
    }

    void FreezePowerUp::update(sf::Time deltaTime)
    {
        // DO NOT call base class update - implement everything here
        if (!updateLifetime(deltaTime))
            return;

        m_pulseAnimation += deltaTime.asSeconds() * 2.0f;
        m_position.y = m_baseY + computeBobbingOffset();
        if (getSpriteComponent())
            getSpriteComponent()->update(deltaTime);
    }

    void FreezePowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        DrawUtils::drawSpriteIfPresent(*this, target, states);
    }

    // ExtraLifePowerUp implementation
    ExtraLifePowerUp::ExtraLifePowerUp()
        : PowerUp(PowerUpType::ExtraLife, sf::Time::Zero)
        , m_heartbeatAnimation(0.0f)
    {
    }

    void ExtraLifePowerUp::update(sf::Time deltaTime)
    {
        // DO NOT call base class update - implement everything here
        if (!updateLifetime(deltaTime))
            return;

        m_heartbeatAnimation += deltaTime.asSeconds() * m_heartbeatSpeed;
        m_pulseAnimation += deltaTime.asSeconds() * 3.0f;
        m_position.y = m_baseY + computeBobbingOffset();
        if (getSpriteComponent())
        {
            getSpriteComponent()->update(deltaTime);
            float scale = 1.0f + 0.2f * std::sin(m_heartbeatAnimation);
            getSpriteComponent()->setScale(scale, scale);
        }
    }

    void ExtraLifePowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        DrawUtils::drawSpriteIfPresent(*this, target, states);
    }

    // SpeedBoostPowerUp implementation
    SpeedBoostPowerUp::SpeedBoostPowerUp()
        : PowerUp(PowerUpType::SpeedBoost, sf::seconds(m_boostDuration))
        , m_lineAnimation(0.0f)
    {
    }

    void SpeedBoostPowerUp::update(sf::Time deltaTime)
    {
        // DO NOT call base class update - implement everything here
        if (!updateLifetime(deltaTime))
            return;

        m_lineAnimation += deltaTime.asSeconds() * 5.0f;
        m_pulseAnimation += deltaTime.asSeconds() * 4.0f;
        m_position.y = m_baseY + computeBobbingOffset(1.5f);
        if (getSpriteComponent())
            getSpriteComponent()->update(deltaTime);
    }

    void SpeedBoostPowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        DrawUtils::drawSpriteIfPresent(*this, target, states);
    }
}