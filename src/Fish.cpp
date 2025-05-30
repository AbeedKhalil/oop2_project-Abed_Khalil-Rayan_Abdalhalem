#include "Fish.h"
#include "CollisionDetector.h"
#include "GameConstants.h"
#include "Player.h"
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

    void Fish::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

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

    void Fish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_shape, states);
    }

    void Fish::updateVisual()
    {
        m_shape.setFillColor(m_baseColor);
        m_shape.setOutlineColor(m_outlineColor);
        m_shape.setOutlineThickness(m_outlineThickness);
    }

    void Fish::updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
        const Entity* player, sf::Time deltaTime)
    {
        // Only medium and large fish have AI behavior
        if (m_size == FishSize::Small)
            return;

        // Check if we should flee from the player
        if (player && player->isAlive())
        {
            const Player* playerPtr = dynamic_cast<const Player*>(player);
            if (playerPtr)
            {
                float distance = CollisionDetector::getDistance(m_position, player->getPosition());

                // Check if player is larger and within flee range
                FishSize playerSize = playerPtr->getCurrentFishSize();

                // Fix: Medium fish flee from Medium/Large players, Large fish flee from Large players
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
                    // Flee from player - move in opposite direction
                    sf::Vector2f fleeDirection = m_position - player->getPosition();
                    setDirection(fleeDirection.x, fleeDirection.y);
                    return; // Don't hunt if we're fleeing
                }
            }
        }

        // If not fleeing, hunt for smaller prey
        const Entity* closestPrey = nullptr;
        float closestDistance = std::numeric_limits<float>::max();

        // Check if player is close and can be eaten
        if (player && player->isAlive())
        {
            const Player* playerPtr = dynamic_cast<const Player*>(player);
            if (playerPtr)
            {
                FishSize playerSize = playerPtr->getCurrentFishSize();

                // Check if this fish can eat the player
                bool canEatPlayer = false;
                if (m_size == FishSize::Medium)
                {
                    canEatPlayer = (playerSize == FishSize::Small);
                }
                else if (m_size == FishSize::Large)
                {
                    canEatPlayer = (playerSize == FishSize::Small || playerSize == FishSize::Medium);
                }

                if (canEatPlayer)
                {
                    float distance = CollisionDetector::getDistance(m_position, player->getPosition());
                    if (distance < AI_DETECTION_RANGE && distance < closestDistance)
                    {
                        closestDistance = distance;
                        closestPrey = player;
                    }
                }
            }
        }

        // Check other fish entities for prey
        for (const auto& entity : entities)
        {
            if (entity.get() == this || !entity->isAlive())
                continue;

            // Check if it's a smaller fish we can eat
            if (canEat(*entity))
            {
                float distance = CollisionDetector::getDistance(m_position, entity->getPosition());
                if (distance < closestDistance && distance < AI_DETECTION_RANGE)
                {
                    closestDistance = distance;
                    closestPrey = entity.get();
                }
            }
        }

        // Follow the closest prey if found
        if (closestPrey)
        {
            sf::Vector2f direction = closestPrey->getPosition() - m_position;
            setDirection(direction.x, direction.y);
        }
        // Otherwise continue in current direction (set by spawner)
    }

    bool Fish::canEat(const Entity& other) const
    {
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