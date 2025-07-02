#include "Pufferfish.h"
#include "CollisionDetector.h"
#include "GameConstants.h"
#include "Player.h"
#include "SpriteManager.h"
#include "Animator.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <ranges>
#include <execution>

namespace FishGame
{
    // Pufferfish implementation
    Pufferfish::Pufferfish(int currentLevel)
        : AdvancedFish(FishSize::Medium, 100.0f, currentLevel, MovementPattern::Sinusoidal)
        , m_isPuffed(false)
        , m_stateTimer(sf::Time::Zero)
        , m_inflationLevel(0.0f)
        , m_normalRadius(25.0f)
        , m_isPuffing(false)
        , m_puffTimer(sf::Time::Zero)
    {
        m_radius = m_normalRadius;

        // Create spikes
        m_spikes.reserve(m_spikeCount);
        std::generate_n(std::back_inserter(m_spikes), m_spikeCount, [] {
            sf::CircleShape spike(3.0f, 3);  // Triangle
            spike.setFillColor(sf::Color(150, 100, 50));
            spike.setOrigin(3.0f, 3.0f);
            return spike;
            });
    }

    void Pufferfish::initializeSprite(SpriteManager& spriteManager)
    {
        const sf::Texture& tex = spriteManager.getTexture(getTextureID());
        m_animator = std::make_unique<Animator>(createPufferfishAnimator(tex));

        float scale = spriteManager.getScaleConfig().medium;
        m_animator->setScale({ scale, scale });
        m_animator->setPosition(m_position);
        setRenderMode(RenderMode::Sprite);

        m_facingRight = m_velocity.x > 0.f;
        m_currentAnimation = m_facingRight ? "swimRight" : "swimLeft";
        m_animator->play(m_currentAnimation);

    }

    void Pufferfish::update(sf::Time deltaTime)
    {
        // Check base frozen state first
        if (m_isFrozen)
        {
            // Just update position with frozen velocity
            updateMovement(deltaTime);

            // Still update visual elements but not state transitions
            auto spikeIdx = std::views::iota(size_t{ 0 }, m_spikes.size());
            std::for_each(std::execution::unseq, spikeIdx.begin(), spikeIdx.end(),
                [this](size_t i)
                {
                    float angle = (360.0f / m_spikeCount) * i * Constants::DEG_TO_RAD;
                    float spikeRadius = m_radius + (m_inflationLevel * 10.0f);

                    sf::Vector2f spikePos(
                        m_position.x + std::cos(angle) * spikeRadius,
                        m_position.y + std::sin(angle) * spikeRadius);

                    m_spikes[i].setPosition(spikePos);
                    m_spikes[i].setRotation(angle * Constants::RAD_TO_DEG);
                });
            return;
        }

        // Call base class update (which now also checks frozen state)
        AdvancedFish::update(deltaTime);

        if (!m_isAlive)
            return;

        // Update automatic cycle between normal and puffed states
        updateCycleState(deltaTime);

        if (m_puffPhase == PuffPhase::Inflating || m_puffPhase == PuffPhase::Deflating)
        {
            m_puffTimer += deltaTime;
            if (m_puffTimer.asSeconds() >= m_puffAnimDuration)
            {
                m_puffTimer = sf::Time::Zero;
                if (m_puffPhase == PuffPhase::Inflating)
                {
                    m_puffPhase = PuffPhase::Holding;
                    m_isPuffing = false;
                }
                else if (m_puffPhase == PuffPhase::Deflating)
                {
                    m_isPuffing = false;
                    if (m_inflationLevel <= 0.0f)
                        transitionToNormal();
                }
            }
        }

        // Update spike positions
        auto spikeIdx2 = std::views::iota(size_t{ 0 }, m_spikes.size());
        std::for_each(std::execution::unseq, spikeIdx2.begin(), spikeIdx2.end(),
            [this](size_t i)
            {
                float angle = (360.0f / m_spikeCount) * i * Constants::DEG_TO_RAD;
                float spikeRadius = m_radius + (m_inflationLevel * 10.0f);

                sf::Vector2f spikePos(
                    m_position.x + std::cos(angle) * spikeRadius,
                    m_position.y + std::sin(angle) * spikeRadius);

                m_spikes[i].setPosition(spikePos);
                m_spikes[i].setRotation(angle * Constants::RAD_TO_DEG);
            });
    }

    bool Pufferfish::canEat(const Entity& other) const
    {
        // Pufferfish can't eat when inflated
        if (isInflated())
            return false;

        return Fish::canEat(other);
    }

    void Pufferfish::pushEntity(Entity& entity)
    {
        if (!isInflated() || !canPushEntity(entity))
            return;

        // Calculate push direction
        sf::Vector2f pushDirection = entity.getPosition() - m_position;
        float distance = std::sqrt(pushDirection.x * pushDirection.x + pushDirection.y * pushDirection.y);

        if (distance > 0.0f)
        {
            // Normalize and apply push
            pushDirection /= distance;
            sf::Vector2f pushVelocity = pushDirection * m_pushForce;

            // Set entity velocity to push velocity
            entity.setVelocity(pushVelocity);

            // Move entity immediately by push distance
            sf::Vector2f newPosition = entity.getPosition() + pushDirection * m_pushDistance;
            entity.setPosition(newPosition);
        }
    }

    bool Pufferfish::canPushEntity(const Entity& entity) const
    {
        if (!isInflated())
            return false;

        float distance = EntityUtils::distance(*this, entity);
        float pushRadius = m_radius + 10.0f; // Slightly larger than actual radius for push effect

        return distance < pushRadius;
    }

    void Pufferfish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        Fish::draw(target, states);

        // Draw spikes when inflating
        if (m_inflationLevel > 0.2f)
        {
            std::for_each(m_spikes.begin(), m_spikes.end(),
                [&target, &states](const sf::CircleShape& spike)
                {
                    target.draw(spike, states);
                });
        }
    }


    void Pufferfish::updateCycleState(sf::Time deltaTime)
    {
        m_stateTimer += deltaTime;

        if (!m_isPuffed)
        {
            // Normal state
            if (m_stateTimer.asSeconds() >= m_normalStateDuration)
            {
                // Transition to puffed state
                transitionToInflated();
            }
            else if (m_inflationLevel > 0.0f)
            {
                // Continue deflating back to normal size
                m_inflationLevel = std::max(0.0f, m_inflationLevel - m_deflationSpeed * deltaTime.asSeconds());
                m_radius = m_normalRadius * (1.0f + m_inflationLevel * (m_inflatedRadiusMultiplier - 1.0f));
            }
        }
        else
        {
            switch (m_puffPhase)
            {
            case PuffPhase::Inflating:
                if (m_inflationLevel < 1.0f)
                {
                    m_inflationLevel = std::min(1.0f, m_inflationLevel + m_inflationSpeed * deltaTime.asSeconds());
                    m_radius = m_normalRadius * (1.0f + m_inflationLevel * (m_inflatedRadiusMultiplier - 1.0f));
                }
                break;
            case PuffPhase::Holding:
                if (m_stateTimer.asSeconds() >= m_puffedStateDuration)
                {
                    m_puffPhase = PuffPhase::Deflating;
                    m_puffTimer = sf::Time::Zero;
                    if (m_animator)
                    {
                        std::string anim = m_facingRight ? "puffDeflateRight" : "puffDeflateLeft";
                        m_animator->play(anim);
                        m_currentAnimation = anim;
                    }
                }
                break;
            case PuffPhase::Deflating:
                if (m_inflationLevel > 0.0f)
                {
                    m_inflationLevel = std::max(0.0f, m_inflationLevel - m_deflationSpeed * deltaTime.asSeconds());
                    m_radius = m_normalRadius * (1.0f + m_inflationLevel * (m_inflatedRadiusMultiplier - 1.0f));
                }
                break;
            default:
                break;
            }

            if (m_puffPhase == PuffPhase::Deflating && m_inflationLevel <= 0.0f)
            {
                // Wait for animation completion to switch state
            }
        }

        // Slow down when inflated
        float speedMultiplier = 1.0f - (m_inflationLevel * 0.7f);
        m_speed = 100.0f * speedMultiplier;
    }

    void Pufferfish::transitionToInflated()
    {
        m_isPuffed = true;
        m_stateTimer = sf::Time::Zero;
        m_isPuffing = true;
        m_puffTimer = sf::Time::Zero;
        m_puffPhase = PuffPhase::Inflating;
        m_turning = false;
        m_turnTimer = sf::Time::Zero;

        if (m_animator)
        {
            std::string anim = m_facingRight ? "puffInflateRight" : "puffInflateLeft";
            m_animator->play(anim);
            m_currentAnimation = anim;
        }
    }

    void Pufferfish::transitionToNormal()
    {
        m_isPuffed = false;
        m_stateTimer = sf::Time::Zero;
        m_puffPhase = PuffPhase::None;
        if (m_animator)
        {
            std::string anim = m_facingRight ? "swimRight" : "swimLeft";
            m_animator->play(anim);
            m_currentAnimation = anim;
        }
    }

}

