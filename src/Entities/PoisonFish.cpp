#include "PoisonFish.h"
#include "CollisionDetector.h"
#include "GameConstants.h"
#include "Player.h"
#include "SpriteManager.h"
#include "Animator.h"
#include <cmath>

namespace FishGame
{
    // PoisonFish implementation
    PoisonFish::PoisonFish(int currentLevel)
        : AdvancedFish(FishSize::Small, 130.0f, currentLevel, MovementPattern::Sinusoidal)
        , m_wobbleAnimation(0.0f)
        , m_poisonDuration(sf::seconds(m_poisonEffectDuration))
        , m_poisonPoints(m_basePoisonPoints* currentLevel)
    {
        // Purple poisonous appearance
        m_pointValue = 0;

        // Set amplitude for sinusoidal movement - reduced for smaller fish
        m_amplitude = 15.0f;
        m_frequency = 3.0f;

    }

    void PoisonFish::update(sf::Time deltaTime)
    {
        // Call base class update (which handles frozen state)
        AdvancedFish::update(deltaTime);

        if (!m_isAlive)
            return;

        // Update visual effects even when frozen (but slower)
        if (m_isFrozen)
        {
            // Slow down animation when frozen
            m_wobbleAnimation += deltaTime.asSeconds() * 0.3f;
        }
        else
        {
            // Normal animation speed
            m_wobbleAnimation += deltaTime.asSeconds() * 3.0f;
        }

    }

    void PoisonFish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        Fish::draw(target, states);
    }

}

