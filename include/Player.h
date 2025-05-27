// Player.h
#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>
#include <memory>

namespace FishGame
{
    class Player : public Entity
    {
    public:
        Player();
        ~Player() override = default;

        // Entity interface implementation
        void update(sf::Time deltaTime) override;
        sf::FloatRect getBounds() const override;
        EntityType getType() const override { return EntityType::Player; }

        // Player-specific methods
        void handleInput();
        void followMouse(const sf::Vector2f& mousePosition);

        void grow();
        void resetSize();
        int getCurrentStage() const { return m_currentStage; }

        void setWindowBounds(const sf::Vector2u& windowSize);

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updateStage();
        void constrainToWindow();

    private:
        sf::CircleShape m_shape;
        int m_score;
        int m_currentStage;

        // Control state
        bool m_useMouseControl;
        sf::Vector2f m_targetPosition;

        // Movement parameters
        static constexpr float m_baseSpeed = 400.0f;
        static constexpr float m_acceleration = 10.0f;
        static constexpr float m_deceleration = 8.0f;
        static constexpr float m_maxSpeed = 600.0f;

        // Size parameters
        static constexpr float m_baseRadius = 20.0f;
        static constexpr float m_growthFactor = 1.5f;

        // Window bounds
        sf::Vector2u m_windowBounds;

        // Stage thresholds
        static constexpr int m_stage1Threshold = 0;
        static constexpr int m_stage2Threshold = 33;
        static constexpr int m_stage3Threshold = 66;
        static constexpr int m_maxScore = 100;
    };
}