#include "Entity.h"

namespace FishGame
{
    Entity::Entity()
        : m_position(0.0f, 0.0f)
        , m_velocity(0.0f, 0.0f)
        , m_radius(0.0f)
        , m_isAlive(true)
    {
    }

    void Entity::setPosition(float x, float y)
    {
        m_position.x = x;
        m_position.y = y;
    }

    void Entity::setPosition(const sf::Vector2f& position)
    {
        m_position = position;
    }

    sf::Vector2f Entity::getPosition() const
    {
        return m_position;
    }

    void Entity::setVelocity(float vx, float vy)
    {
        m_velocity.x = vx;
        m_velocity.y = vy;
    }

    void Entity::setVelocity(const sf::Vector2f& velocity)
    {
        m_velocity = velocity;
    }

    sf::Vector2f Entity::getVelocity() const
    {
        return m_velocity;
    }

    void Entity::updateMovement(sf::Time deltaTime)
    {
        m_position += m_velocity * deltaTime.asSeconds();
    }
}