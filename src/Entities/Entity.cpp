#include "Entity.h"
#include "SpriteManager.h"

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

    void Entity::setupSprite(SpriteManager& spriteManager, TextureID textureId)
    {
        auto sprite = spriteManager.createSpriteComponent(
            static_cast<Entity*>(this), textureId);
        if (sprite)
        {
            auto config = spriteManager.getSpriteConfig<Entity>(textureId);
            sprite->configure(config);
            setSpriteComponent(std::move(sprite));
            setRenderMode(RenderMode::Sprite);
        }
    }
}
