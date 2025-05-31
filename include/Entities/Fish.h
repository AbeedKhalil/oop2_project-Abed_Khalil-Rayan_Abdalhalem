#pragma once

#include "Entity.h"
#include <vector>
#include <memory>

namespace FishGame
{
    // Fish size categories for gameplay mechanics
    enum class FishSize
    {
        Small,
        Medium,
        Large
    };

    // Base class for all fish entities
    class Fish : public Entity
    {
    public:
        Fish(FishSize size, float speed, int currentLevel);
        virtual ~Fish() = default;

        // Entity interface implementation
        void update(sf::Time deltaTime) override;
        sf::FloatRect getBounds() const override;

        // Fish-specific methods
        FishSize getSize() const { return m_size; }
        float getSpeed() const { return m_speed; }
        int getCurrentLevel() const { return m_currentLevel; }
        sf::Vector2u getWindowBounds() const { return m_windowBounds; }

        // Virtual methods for derived classes
        virtual int getPointValue() const { return m_pointValue; }
        virtual bool canEat(const Entity& other) const;
        virtual void updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
            const Entity* player, sf::Time deltaTime);

        void setDirection(float dirX, float dirY);
        void setWindowBounds(const sf::Vector2u& windowSize);

        // Static utility method
        static int getPointValue(FishSize size, int level);

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        virtual void updateVisual();
        void updateMovement(sf::Time deltaTime);

    protected:
        sf::CircleShape m_shape;
        FishSize m_size;
        float m_speed;
        int m_pointValue;
        int m_currentLevel;
        sf::Vector2u m_windowBounds;

        // Visual properties
        sf::Color m_baseColor;
        sf::Color m_outlineColor;
        float m_outlineThickness;
    };
}