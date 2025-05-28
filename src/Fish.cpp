// Fish.cpp
#include "Fish.h"
#include "CollisionDetector.h"
#include <cmath>
#include <algorithm>

namespace FishGame
{
    Fish::Fish(FishSize size, float speed, int pointValue)
        : Entity()
        , m_shape()
        , m_size(size)
        , m_speed(speed)
        , m_pointValue(pointValue)
        , m_windowBounds(1920, 1080)
        , m_baseColor(sf::Color::White)
        , m_outlineColor(sf::Color::Black)
        , m_outlineThickness(1.0f)
    {
        // Set radius based on size
        switch (m_size)
        {
        case FishSize::Small:
            m_radius = 15.0f;
            break;
        case FishSize::Medium:
            m_radius = 25.0f;
            break;
        case FishSize::Large:
            m_radius = 35.0f;
            break;
        }

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
        // Only medium and large fish have following behavior
        if (m_size == FishSize::Small)
            return;

        // Find closest prey (fish or player) within close range
        const Entity* closestPrey = nullptr;
        float closestDistance = std::numeric_limits<float>::max();
        const float detectionRange = 55.0f; // Small detection range - only follow when close

        // Check if player is close and can be targeted
        if (player && player->isAlive())
        {
            float distance = CollisionDetector::getDistance(m_position, player->getPosition());
            if (distance < detectionRange && distance < closestDistance)
            {
                closestDistance = distance;
                closestPrey = player;
            }
        }

        // Check other fish entities
        for (const auto& entity : entities)
        {
            if (entity.get() == this || !entity->isAlive())
                continue;

            // Check if it's a smaller fish we can eat
            if (canEat(*entity))
            {
                float distance = CollisionDetector::getDistance(m_position, entity->getPosition());
                if (distance < closestDistance && distance < detectionRange)
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
}