#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <set>
#include <string>

namespace FishGame
{
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
        virtual ~Entity() = default;

        // Delete copy operations
        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        // Allow move operations
        Entity(Entity&&) = default;
        Entity& operator=(Entity&&) = default;

        // Pure virtual functions
        virtual void update(sf::Time deltaTime) = 0;
        virtual sf::FloatRect getBounds() const = 0;
        virtual EntityType getType() const = 0;

        // Common entity operations
        void setPosition(float x, float y);
        void setPosition(const sf::Vector2f& position);
        sf::Vector2f getPosition() const;

        void setVelocity(float vx, float vy);
        void setVelocity(const sf::Vector2f& velocity);
        sf::Vector2f getVelocity() const;

        bool isAlive() const { return m_isAlive; }
        void destroy() { m_isAlive = false; }

        float getRadius() const { return m_radius; }
        void setRadius(float radius) { m_radius = radius; }

        // Tag system for advanced behaviors
        void addTag(const std::string& tag) { m_tags.insert(tag); }
        void removeTag(const std::string& tag) { m_tags.erase(tag); }
        bool hasTag(const std::string& tag) const { return m_tags.find(tag) != m_tags.end(); }
        const std::set<std::string>& getTags() const { return m_tags; }

    protected:
        // Protected draw function for derived classes
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override = 0;

        // Update position based on velocity
        void updateMovement(sf::Time deltaTime);

    protected:
        sf::Vector2f m_position;
        sf::Vector2f m_velocity;
        float m_radius;
        bool m_isAlive;

        // Tag system for flexible entity behaviors
        std::set<std::string> m_tags;
    };
}