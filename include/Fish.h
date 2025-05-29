// Fish.h
#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

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
        ~Fish() override = default;

        // Entity interface implementation
        void update(sf::Time deltaTime) override;
        sf::FloatRect getBounds() const override;

        // Fish-specific methods
        FishSize getSize() const { return m_size; }
        int getPointValue() const { return m_pointValue; }
        float getSpeed() const { return m_speed; }

        void setDirection(float dirX, float dirY);
        void setWindowBounds(const sf::Vector2u& windowSize);

        // AI behavior - now includes fleeing
        void updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
            const Entity* player, sf::Time deltaTime);
        bool canEat(const Entity& other) const;

        // Get point value based on fish size and level
        static int getPointValue(FishSize size, int level);

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        // Update visual representation
        virtual void updateVisual();

    protected:
        sf::CircleShape m_shape;
        FishSize m_size;
        float m_speed;
        int m_pointValue;
        int m_currentLevel;

        // Window boundaries for wrapping/destruction
        sf::Vector2u m_windowBounds;

        // Visual properties
        sf::Color m_baseColor;
        sf::Color m_outlineColor;
        float m_outlineThickness;
    };
}