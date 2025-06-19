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
    commonUpdate(deltaTime, 2.0f);
}

void FreezePowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    SpriteDrawable<FreezePowerUp>::draw(target, states);
}



    // ExtraLifePowerUp implementation
    ExtraLifePowerUp::ExtraLifePowerUp()
        : PowerUp(PowerUpType::ExtraLife, sf::Time::Zero)
        , m_heartbeatAnimation(0.0f)
    {
    }

void ExtraLifePowerUp::update(sf::Time deltaTime)
{
    commonUpdate(deltaTime, 3.0f);
    m_heartbeatAnimation += deltaTime.asSeconds() * m_heartbeatSpeed;
    if (auto sprite = getSpriteComponent())
    {
        float scale = 1.0f + 0.2f * std::sin(m_heartbeatAnimation);
        sprite->setScale(scale, scale);
    }
}

void ExtraLifePowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    SpriteDrawable<ExtraLifePowerUp>::draw(target, states);
}



    // SpeedBoostPowerUp implementation
    SpeedBoostPowerUp::SpeedBoostPowerUp()
        : PowerUp(PowerUpType::SpeedBoost, sf::seconds(m_boostDuration))
        , m_lineAnimation(0.0f)
    {
    }

void SpeedBoostPowerUp::update(sf::Time deltaTime)
{
    commonUpdate(deltaTime, 4.0f, 1.5f);
    m_lineAnimation += deltaTime.asSeconds() * 5.0f;
}

void SpeedBoostPowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    SpriteDrawable<SpeedBoostPowerUp>::draw(target, states);
}


}