#include "Entity.h"
#include "SpriteComponent.h" 
#include <algorithm>

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

    void Entity::addTag(std::string_view tag)
    {
        m_tags.emplace(tag);
    }

    void Entity::removeTag(std::string_view tag)
    {
        m_tags.erase(std::string(tag));
    }

    bool Entity::hasTag(std::string_view tag) const
    {
        return m_tags.find(std::string(tag)) != m_tags.end();
    }

    bool Entity::hasAnyTag(std::initializer_list<std::string_view> tags) const
    {
        return std::any_of(tags.begin(), tags.end(),
            [this](std::string_view tag) {
                return hasTag(tag);
            });
    }
}