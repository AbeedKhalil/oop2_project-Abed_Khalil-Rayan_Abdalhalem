#include "Fish.h"
#include "SpriteManager.h"
#include "SpriteComponent.h"
#include "CollisionDetector.h"
#include "GameConstants.h"
#include "Player.h"
#include "SpecialFish.h"
#include <cmath>
#include <algorithm>

namespace FishGame
{
    using namespace Constants;

    Fish::Fish(FishSize size, float speed, int currentLevel)
        : Entity()
        , m_shape()
        , m_size(size)
        , m_speed(speed)
        , m_currentLevel(currentLevel)
        , m_windowBounds(WINDOW_WIDTH, WINDOW_HEIGHT)
        , m_baseColor(sf::Color::White)
        , m_outlineColor(sf::Color::Black)
        , m_outlineThickness(1.0f)
        , m_isFleeing(false)
        , m_fleeSpeed(speed* m_fleeSpeedMultiplier)
        , m_fleeDirection(0.0f, 0.0f)
        , m_isPoisoned(false)
        , m_isStunned(false)
        , m_poisonTimer(sf::Time::Zero)
        , m_stunTimer(sf::Time::Zero)
        , m_originalVelocity(0.0f, 0.0f)
        , m_isFrozen(false)
        , m_velocityBeforeFreeze(0.0f, 0.0f)
        , m_damageFlashTimer(sf::Time::Zero)
    {
        // Set radius based on size
        switch (m_size)
        {
        case FishSize::Small:
            m_radius = SMALL_FISH_RADIUS;
            break;
        case FishSize::Medium:
            m_radius = MEDIUM_FISH_RADIUS;
            break;
        case FishSize::Large:
            m_radius = LARGE_FISH_RADIUS;
            break;
        }

        // Set point value based on size and level
        m_pointValue = getPointValue(m_size, m_currentLevel);

        m_shape.setRadius(m_radius);
        m_shape.setOrigin(m_radius, m_radius);
        updateVisual();
    }

    void Fish::startFleeing()
    {
        if (m_isFleeing)
            return;

        m_isFleeing = true;

        // Determine flee direction based on position
        float centerX = m_windowBounds.x / 2.0f;

        // Flee to nearest edge
        if (m_position.x < centerX)
        {
            m_fleeDirection = sf::Vector2f(-1.0f, 0.0f);  // Flee left
        }
        else
        {
            m_fleeDirection = sf::Vector2f(1.0f, 0.0f);   // Flee right
        }

        // Set velocity for fleeing
        m_velocity = m_fleeDirection * m_fleeSpeed;
    }

    void Fish::updateFleeingBehavior(sf::Time deltaTime)
    {
        if (!m_isFleeing)
            return;

        // Just maintain fleeing velocity
        // The fish will be destroyed when it goes off screen
    }

    void Fish::initializeSprite(SpriteManager& spriteManager)
    {
        // Create sprite component
        auto sprite = spriteManager.createSpriteComponent(
            static_cast<Entity*>(this), getTextureID());

        // Configure sprite based on fish size
        if (sprite)
        {
            auto config = spriteManager.getSpriteConfig<Entity>(getTextureID(), m_size);
            sprite->configure(config);
            setSpriteComponent(std::move(sprite));

            // Set render mode to sprite
            setRenderMode(RenderMode::Sprite);

            // Apply initial visual state
            updateVisualState();
        }
    }

    void Fish::updateVisualState()
    {
        if (m_sprite)
        {
            // Update sprite color based on state
            sf::Color spriteColor = sf::Color::White;

            if (m_isPoisoned)
            {
                spriteColor = sf::Color(200, 100, 255); // Purple tint
            }
            else if (m_isStunned)
            {
                spriteColor = sf::Color(150, 150, 150); // Gray tint
            }
            else if (m_isFrozen)
            {
                spriteColor = sf::Color(150, 200, 255); // Blue tint
            }

            m_sprite->setColor(spriteColor);

            // Apply effects
            if (m_isFleeing)
            {
                m_sprite->applyPulseEffect(0.1f, 5.0f);
            }
        }
    }

    void Fish::updateSpriteEffects(sf::Time deltaTime)
    {
        // Override in derived classes for specific effects
        
        // Example: flash when hit
        if (m_damageFlashTimer > sf::Time::Zero)
        {
            m_damageFlashTimer -= deltaTime;
            float intensity = m_damageFlashTimer.asSeconds() / 0.2f;
            
            if (m_sprite)
            {
                m_sprite->applyFlashEffect(sf::Color::Red, intensity);
            }
        }
    }

    TextureID Fish::getTextureID() const
    {
        // Default implementation - override in derived classes
        switch (m_size)
        {
        case FishSize::Small:
            return TextureID::SmallFish;
        case FishSize::Medium:
            return TextureID::MediumFish;
        case FishSize::Large:
            return TextureID::LargeFish;
        default:
            return TextureID::SmallFish;
        }
    }

    void Fish::setFrozen(bool frozen)
    {
        if (frozen && !m_isFrozen)
        {
            // Entering frozen state
            m_isFrozen = true;
            m_velocityBeforeFreeze = m_velocity;
            m_velocity = m_velocityBeforeFreeze * 0.1f; // 90% speed reduction
        }
        else if (!frozen && m_isFrozen)
        {
            // Exiting frozen state
            m_isFrozen = false;
            // Restore original direction but with current speed
            float currentSpeed = std::sqrt(m_velocityBeforeFreeze.x * m_velocityBeforeFreeze.x +
                m_velocityBeforeFreeze.y * m_velocityBeforeFreeze.y);
            if (currentSpeed > 0.0f)
            {
                sf::Vector2f direction = m_velocityBeforeFreeze / currentSpeed;
                m_velocity = direction * m_speed;
            }
        }
    }

    void Fish::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        // Handle stunned state
        if (m_isStunned)
        {
            m_stunTimer -= deltaTime;
            if (m_stunTimer <= sf::Time::Zero)
            {
                m_isStunned = false;
                m_velocity = m_originalVelocity;
            }
            else
            {
                m_velocity = sf::Vector2f(0.0f, 0.0f);
            }
        }

        // Handle poisoned state
        if (m_isPoisoned && !m_isStunned)
        {
            m_poisonTimer -= deltaTime;
            if (m_poisonTimer <= sf::Time::Zero)
            {
                m_isPoisoned = false;
                // Reverse velocity back to normal
                m_velocity = -m_velocity;
            }
        }

        // Update fleeing behavior if active
        if (m_isFleeing)
        {
            updateFleeingBehavior(deltaTime);
        }

        // Update position
        updateMovement(deltaTime);

        // Check if fish has moved off screen
        if (m_velocity.x > 0 && m_position.x > m_windowBounds.x + m_radius)
        {
            destroy();
        }
        else if (m_velocity.x < 0 && m_position.x < -m_radius)
        {
            destroy();
        }

        // Update visual position
        m_shape.setPosition(m_position);

        // Update sprite if present
        if (m_sprite && m_renderMode == RenderMode::Sprite)
        {
            m_sprite->update(deltaTime);
            updateSpriteEffects(deltaTime);
        }

        // Update visual position (both circle and sprite)
        m_shape.setPosition(m_position);
    }

    sf::FloatRect Fish::getBounds() const
    {
        return sf::FloatRect(m_position.x - m_radius, m_position.y - m_radius,
            m_radius * 2.0f, m_radius * 2.0f);
    }

    void Fish::setDirection(float dirX, float dirY)
    {
        float length = std::sqrt(dirX * dirX + dirY * dirY);
        if (length > 0.0f)
        {
            m_velocity = sf::Vector2f(dirX / length * m_speed, dirY / length * m_speed);
        }
    }

    void Fish::setWindowBounds(const sf::Vector2u& windowSize)
    {
        m_windowBounds = windowSize;
    }

    void Fish::setPoisoned(sf::Time duration)
    {
        if (!m_isPoisoned && !m_isStunned)
        {
            m_isPoisoned = true;
            m_poisonTimer = duration;
            // Reverse movement direction
            m_velocity = -m_velocity;
        }
    }

    void Fish::setStunned(sf::Time duration)
    {
        if (!m_isStunned)
        {
            m_isStunned = true;
            m_stunTimer = duration;
            m_originalVelocity = m_velocity;
            m_velocity = sf::Vector2f(0.0f, 0.0f);
        }
    }

    bool Fish::canEat(const Entity& other) const
    {
        // Check if it's a player
        if (other.getType() == EntityType::Player)
        {
            const Player* player = dynamic_cast<const Player*>(&other);
            if (!player)
                return false;

            // Fish can eat player if fish is larger than player's current size
            FishSize playerSize = player->getCurrentFishSize();
            return static_cast<int>(m_size) > static_cast<int>(playerSize);
        }

        // Check if it's another fish
        if (other.getType() != EntityType::SmallFish &&
            other.getType() != EntityType::MediumFish &&
            other.getType() != EntityType::LargeFish)
        {
            return false;
        }

        // Cast to Fish to get size
        const Fish* otherFish = dynamic_cast<const Fish*>(&other);
        if (!otherFish)
            return false;

        // Can eat smaller fish
        return static_cast<int>(m_size) > static_cast<int>(otherFish->getSize());
    }

    void Fish::updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
        const Entity* player, sf::Time deltaTime)
    {
        // Skip AI if frozen, fleeing, or stunned
        if (m_isFrozen || m_isFleeing || m_isStunned)
            return;

        // Default AI behavior - only medium and large fish have AI behavior by default
        if (m_size == FishSize::Small)
            return;

        // Check if we should flee from the player
        if (player && player->isAlive())
        {
            const Player* playerPtr = dynamic_cast<const Player*>(player);
            if (playerPtr)
            {
                float distance = EntityUtils::distance(*this, *player);

                // Check if player is larger and within flee range
                FishSize playerSize = playerPtr->getCurrentFishSize();

                bool shouldFlee = false;
                if (m_size == FishSize::Medium)
                {
                    shouldFlee = (playerSize == FishSize::Medium || playerSize == FishSize::Large);
                }
                else if (m_size == FishSize::Large)
                {
                    shouldFlee = (playerSize == FishSize::Large);
                }

                if (shouldFlee && distance < AI_FLEE_RANGE)
                {
                    // Flee from player
                    sf::Vector2f fleeDirection = m_position - player->getPosition();
                    setDirection(fleeDirection.x, fleeDirection.y);
                    return;
                }

                // Check if we can hunt the player
                if (canEat(*player) && distance < AI_DETECTION_RANGE)
                {
                    // Hunt the player
                    sf::Vector2f huntDirection = player->getPosition() - m_position;
                    setDirection(huntDirection.x, huntDirection.y);
                    return;
                }
            }
        }

        // NEW: Check for puffed pufferfish to avoid
        std::for_each(entities.begin(), entities.end(),
            [this](const std::unique_ptr<Entity>& entity)
            {
                if (auto* pufferfish = dynamic_cast<const Pufferfish*>(entity.get()))
                {
                    if (pufferfish->isInflated() && pufferfish->isAlive())
                    {
                        float distance = EntityUtils::distance(*this, *pufferfish);

                        // Avoid puffed pufferfish
                        if (distance < AI_FLEE_RANGE * 1.5f)
                        {
                            sf::Vector2f avoidDirection = m_position - pufferfish->getPosition();
                            setDirection(avoidDirection.x, avoidDirection.y);
                            return;
                        }
                    }
                }
            });

        // Hunt for smaller prey
        const Entity* closestPrey = nullptr;
        float closestDistance = std::numeric_limits<float>::max();

        // Use STL algorithms to find closest prey
        std::for_each(entities.begin(), entities.end(),
            [this, &closestPrey, &closestDistance](const std::unique_ptr<Entity>& entity)
            {
                if (entity.get() != this && entity->isAlive() && canEat(*entity))
                {
                    // Skip puffed pufferfish
                    if (auto* pufferfish = dynamic_cast<const Pufferfish*>(entity.get()))
                    {
                        if (pufferfish->isInflated())
                            return;
                    }

                    float distance = EntityUtils::distance(*this, *entity);
                    if (distance < closestDistance && distance < AI_DETECTION_RANGE)
                    {
                        closestDistance = distance;
                        closestPrey = entity.get();
                    }
                }
            });

        // Follow the closest prey if found
        if (closestPrey)
        {
            sf::Vector2f direction = closestPrey->getPosition() - m_position;
            setDirection(direction.x, direction.y);
        }
    }

    void Fish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (m_renderMode == RenderMode::Sprite && m_sprite)
        {
            target.draw(*m_sprite, states);
        }
        else
        {
            // Fallback to circle rendering
            target.draw(m_shape, states);
        }
    }

    void Fish::updateVisual()
    {
        m_shape.setFillColor(m_baseColor);
        m_shape.setOutlineColor(m_outlineColor);
        m_shape.setOutlineThickness(m_outlineThickness);
    }

    void Fish::updateMovement(sf::Time deltaTime)
    {
        m_position += m_velocity * deltaTime.asSeconds();
    }

    int Fish::getPointValue(FishSize size, int level)
    {
        if (level == 1)
        {
            switch (size)
            {
            case FishSize::Small: return 3;
            case FishSize::Medium: return 6;
            case FishSize::Large: return 9;
            }
        }
        else // Level 2 and above
        {
            switch (size)
            {
            case FishSize::Small: return 2;
            case FishSize::Medium: return 4;
            case FishSize::Large: return 8;
            }
        }
        return 1; // Default
    }
}