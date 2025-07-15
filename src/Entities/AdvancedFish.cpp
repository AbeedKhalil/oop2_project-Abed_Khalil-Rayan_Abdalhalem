#include "AdvancedFish.h"

namespace FishGame
{
    // AdvancedFish implementation
    AdvancedFish::AdvancedFish(FishSize size, float speed, int currentLevel, MovementPattern pattern)
        : Fish(size, speed, currentLevel)
        , m_movementPattern(pattern)
        , m_patternTimer(0.0f)
        , m_baseY(0.0f)
        , m_amplitude(30.0f)
        , m_frequency(2.0f)
    {
    }

    void AdvancedFish::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        // When frozen simply defer to base update so the sprite stays synced
        if (m_isFrozen)
        {
            Fish::update(deltaTime);
            return;
        }

        m_patternTimer += deltaTime.asSeconds();

        // Update movement pattern which may modify velocity/position
        updateMovementPattern(deltaTime);

        // Let the base class handle movement, sprite sync and bounds checks
        Fish::update(deltaTime);
    }

    void AdvancedFish::updateMovementPattern(sf::Time /*deltaTime*/)
    {
        switch (m_movementPattern)
        {
        case MovementPattern::Sinusoidal:
        {
            // Store base Y on first frame
            if (m_baseY == 0.0f)
                m_baseY = m_position.y;

            // Apply sinusoidal movement
            float yOffset = m_amplitude * std::sin(m_patternTimer * m_frequency);
            m_position.y = m_baseY + yOffset;
            break;
        }

        case MovementPattern::ZigZag:
        {
            // Change Y direction periodically
            float zigzagPeriod = 1.0f;
            if (std::fmod(m_patternTimer, zigzagPeriod) < zigzagPeriod / 2)
            {
                m_velocity.y = m_speed * 0.5f;
            }
            else
            {
                m_velocity.y = -m_speed * 0.5f;
            }
            break;
        }

        case MovementPattern::Linear:
        default:
            // No special movement
            break;
        }
    }
}

