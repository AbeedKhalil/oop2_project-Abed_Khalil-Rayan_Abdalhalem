// Player.h
#pragma once

#include "Entity.h"
#include "Fish.h"
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

        void grow(int points);
        void resetSize();
        int getCurrentStage() const { return m_currentStage; }
        int getScore() const { return m_score; }

        // Size comparison for eating mechanics
        bool canEat(const Entity& other) const;
        FishSize getCurrentFishSize() const;

        // Life management
        void die();
        void respawn();
        bool isInvulnerable() const { return m_invulnerabilityTimer > sf::Time::Zero; }

        void setWindowBounds(const sf::Vector2u& windowSize);

        // Get the starting score for a given stage
        static int getStageStartingScore(int stage);

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updateStage();
        void constrainToWindow();
        void updateInvulnerability(sf::Time deltaTime);

    private:
        sf::CircleShape m_shape;
        int m_score;
        int m_currentStage;

        // Control state
        bool m_useMouseControl;
        sf::Vector2f m_targetPosition;

        // Invulnerability after respawn
        sf::Time m_invulnerabilityTimer;
        static const sf::Time m_invulnerabilityDuration;

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