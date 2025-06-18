#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

namespace FishGame
{
    // Forward declarations
    template<typename T> class SpriteComponent;
    enum class TextureID;

    // Entity types for identification
    enum class EntityType
    {
        None,
        Player,
        SmallFish,
        MediumFish,
        LargeFish,
        PowerUp,
        Hazard
    };

    // Base class for all game entities
    class Entity : public sf::Drawable
    {
    public:
        Entity();
        virtual ~Entity(); // Must be defined in .cpp for unique_ptr with incomplete type

        // Delete copy operations
        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        // Allow move operations
        Entity(Entity&&) = default;
        Entity& operator=(Entity&&) = default;

        // Pure virtual functions - MUST be implemented by derived classes
        virtual void update(sf::Time deltaTime) = 0;
        virtual sf::FloatRect getBounds() const = 0;
        virtual EntityType getType() const = 0;

        // Position management
        void setPosition(float x, float y) noexcept { m_position = { x, y }; }
        void setPosition(const sf::Vector2f& position) noexcept { m_position = position; }
        const sf::Vector2f& getPosition() const noexcept { return m_position; }

        // Velocity management
        void setVelocity(float vx, float vy) noexcept { m_velocity = { vx, vy }; }
        void setVelocity(const sf::Vector2f& velocity) noexcept { m_velocity = velocity; }
        const sf::Vector2f& getVelocity() const noexcept { return m_velocity; }

        // Lifecycle management
        bool isAlive() const noexcept { return m_isAlive; }
        void destroy() noexcept { m_isAlive = false; }

        // Collision helpers
        float getRadius() const noexcept { return m_radius; }
        void setRadius(float radius) noexcept { m_radius = radius; }

        // Common movement update
        void updatePosition(sf::Time deltaTime) noexcept;

        // Sprite support - declared here, defined in Entity.cpp
        void setSpriteComponent(std::unique_ptr<SpriteComponent<Entity>> sprite);
        SpriteComponent<Entity>* getSpriteComponent();
        const SpriteComponent<Entity>* getSpriteComponent() const;

        // Visual mode
        enum class RenderMode { Circle, Sprite };
        void setRenderMode(RenderMode mode) { m_renderMode = mode; }
        RenderMode getRenderMode() const { return m_renderMode; }

    protected:
        // Protected draw function for derived classes
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override = 0;

        // Common update pattern for derived classes
        void updateMovement(sf::Time deltaTime) noexcept { updatePosition(deltaTime); }

    protected:
        sf::Vector2f m_position{ 0.0f, 0.0f };
        sf::Vector2f m_velocity{ 0.0f, 0.0f };
        float m_radius{ 0.0f };
        bool m_isAlive{ true };

        // Sprite component - using unique_ptr requires complete type in .cpp
        std::unique_ptr<SpriteComponent<Entity>> m_sprite;
        RenderMode m_renderMode = RenderMode::Sprite;
    };

    // Utility functions for entity operations
    namespace EntityUtils
    {
        // Calculate distance squared between two entities (more efficient than distance)
        inline float distanceSquared(const Entity& a, const Entity& b) noexcept
        {
            const auto diff = a.getPosition() - b.getPosition();
            return diff.x * diff.x + diff.y * diff.y;
        }

        // Calculate actual distance between two entities
        inline float distance(const Entity& a, const Entity& b) noexcept
        {
            return std::sqrt(distanceSquared(a, b));
        }

        // Check if two entities are colliding (circle collision)
        inline bool areColliding(const Entity& a, const Entity& b) noexcept
        {
            const float radiusSum = a.getRadius() + b.getRadius();
            return distanceSquared(a, b) < (radiusSum * radiusSum);
        }

        // Apply a function to all alive entities in a container
        template<typename Container, typename Func>
        void forEachAlive(Container& entities, Func&& func)
        {
            std::for_each(entities.begin(), entities.end(),
                [&func](auto& entity) {
                    if (entity && entity->isAlive())
                    {
                        func(*entity);
                    }
                });
        }

        // Remove all dead entities from a container
        template<typename Container>
        void removeDeadEntities(Container& entities)
        {
            entities.erase(
                std::remove_if(entities.begin(), entities.end(),
                    [](const auto& entity) {
                        return !entity || !entity->isAlive();
                    }),
                entities.end()
            );
        }
    }
}