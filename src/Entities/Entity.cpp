#include "Entity.h"
#include "SpriteComponent.h"

namespace FishGame
{
    Entity::Entity() = default;

    Entity::~Entity() = default;

    void Entity::setSpriteComponent(std::unique_ptr<SpriteComponent<Entity>> sprite)
    {
        m_sprite = std::move(sprite);
    }

    SpriteComponent<Entity>* Entity::getSpriteComponent()
    {
        return m_sprite.get();
    }

    const SpriteComponent<Entity>* Entity::getSpriteComponent() const
    {
        return m_sprite.get();
    }

    void Entity::updatePosition(sf::Time deltaTime) noexcept
    {
        m_position += m_velocity * deltaTime.asSeconds();
    }
}