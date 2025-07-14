#include "PoisonFish.h"
#include "CollisionDetector.h"
#include "GameConstants.h"
#include "Player.h"
#include "SpriteManager.h"
#include "Animator.h"
#include "Systems/CollisionSystem.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <ranges>
#include <execution>

namespace FishGame
{
    // PoisonFish implementation
    PoisonFish::PoisonFish(int currentLevel)
        : AdvancedFish(FishSize::Small, 130.0f, currentLevel, MovementPattern::Sinusoidal)
        , m_poisonBubbles()
        , m_wobbleAnimation(0.0f)
        , m_poisonDuration(sf::seconds(m_poisonEffectDuration))
        , m_poisonPoints(m_basePoisonPoints* currentLevel)
    {
        // Purple poisonous appearance
        m_pointValue = 0;

        // Set amplitude for sinusoidal movement - reduced for smaller fish
        m_amplitude = 15.0f;
        m_frequency = 3.0f;

        // Create poison bubble effects - smaller bubbles for small fish
        m_poisonBubbles.reserve(m_bubbleCount);
        std::generate_n(std::back_inserter(m_poisonBubbles), m_bubbleCount, [] {
            sf::CircleShape bubble(2.0f);
            bubble.setFillColor(sf::Color(200, 100, 255, 150));
            bubble.setOrigin(2.0f, 2.0f);
            return bubble;
            });
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

        // Update poison bubbles
        updatePoisonBubbles(deltaTime);
    }

    void PoisonFish::updatePoisonBubbles(sf::Time /*deltaTime*/)
    {
        auto bubbleIdx = std::views::iota(size_t{ 0 }, m_poisonBubbles.size());
        std::for_each(std::execution::unseq, bubbleIdx.begin(), bubbleIdx.end(),
            [this](size_t i)
            {
                float angle = (60.0f * i + m_wobbleAnimation * 30.0f) * Constants::DEG_TO_RAD;
                float radius = 18.0f + 3.0f * std::sin(m_wobbleAnimation + i);

                sf::Vector2f bubblePos(
                    m_position.x + std::cos(angle) * radius,
                    m_position.y + std::sin(angle) * radius);

                m_poisonBubbles[i].setPosition(bubblePos);

                // Pulsing effect for bubbles
                float scale = 1.0f + 0.2f * std::sin(m_wobbleAnimation * 2.0f + i);
                m_poisonBubbles[i].setScale(scale, scale);
            });
    }

    void PoisonFish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Draw poison bubbles first
        std::for_each(m_poisonBubbles.begin(), m_poisonBubbles.end(),
            [&target, &states](const sf::CircleShape& bubble) {
                target.draw(bubble, states);
            });

        // Draw the fish
        Fish::draw(target, states);
    }

    void PoisonFish::onCollide(Player& player, CollisionSystem& system)
    {
        if (player.isInvulnerable() || system.m_playerStunned)
            return;

        if (player.canEat(*this) && player.attemptEat(*this))
        {
            system.m_reverseControls();
            system.m_controlReverseTimer = getPoisonDuration();
            player.applyPoisonEffect(getPoisonDuration());
            system.m_sounds.play(SoundEffectID::PlayerPoison);
            system.createParticle(getPosition(), sf::Color::Magenta, 15);
            system.createParticle(player.getPosition(), sf::Color::Magenta, 10);
            system.m_levelCounts[getTextureID()]++;
            destroy();
        }
    }

}

