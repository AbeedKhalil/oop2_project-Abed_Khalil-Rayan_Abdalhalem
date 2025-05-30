#pragma once

#include "Fish.h"
#include "GrowthMeter.h"
#include "FrenzySystem.h"
#include "PowerUp.h"
#include "ScoreSystem.h"

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

        // Initialize with game systems
        void initializeSystems(GrowthMeter* growthMeter, FrenzySystem* frenzySystem,
            PowerUpManager* powerUpManager, ScoreSystem* scoreSystem);

        // Player-specific methods
        void handleInput();
        void followMouse(const sf::Vector2f& mousePosition);

        // Growth and size
        void grow(int points);
        void advanceStage();
        void resetSize();
        int getCurrentStage() const { return m_currentStage; }
        int getScore() const { return m_score; }
        float getGrowthProgress() const { return m_growthProgress; }

        // Eating mechanics
        bool canEat(const Entity& other) const;
        bool attemptEat(Entity& other);
        FishSize getCurrentFishSize() const;

        // Tail-bite detection
        bool canTailBite(const Entity& other) const;
        bool attemptTailBite(Entity& other);

        // Life management
        void takeDamage();
        void die();
        void respawn();
        bool isInvulnerable() const { return m_invulnerabilityTimer > sf::Time::Zero; }
        bool hasRecentlyTakenDamage() const { return m_damageCooldown > sf::Time::Zero; }

        // Power-up effects
        void applySpeedBoost(float multiplier, sf::Time duration);
        void applyInvincibility(sf::Time duration);

        // Size information
        bool isAtMaxSize() const { return m_currentStage >= 4; }  // Changed from 3 to 4
        void setWindowBounds(const sf::Vector2u& windowSize);

        // Statistics tracking
        int getTotalFishEaten() const { return m_totalFishEaten; }
        int getDamageTaken() const { return m_damageTaken; }
        bool hasTakenDamage() const { return m_damageTaken > 0; }

        // Visual effects
        void triggerEatEffect();
        void triggerDamageEffect();

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updateStage();
        void constrainToWindow();
        void updateInvulnerability(sf::Time deltaTime);
        void updateVisualEffects(sf::Time deltaTime);
        void handlePredatorBehavior(const Entity& predator);

    private:
        sf::CircleShape m_shape;
        int m_score;
        int m_currentStage;
        float m_growthProgress;

        // Control state
        bool m_useMouseControl;
        sf::Vector2f m_targetPosition;

        // System references
        GrowthMeter* m_growthMeter;
        FrenzySystem* m_frenzySystem;
        PowerUpManager* m_powerUpManager;
        ScoreSystem* m_scoreSystem;

        // Invulnerability and damage
        sf::Time m_invulnerabilityTimer;
        sf::Time m_damageCooldown;
        static const sf::Time m_invulnerabilityDuration;
        static const sf::Time m_damageCooldownDuration;

        // Power-up effects
        float m_speedMultiplier;
        sf::Time m_speedBoostTimer;
        sf::Time m_invincibilityTimer;

        // Movement parameters
        static constexpr float m_baseSpeed = 400.0f;
        static constexpr float m_acceleration = 10.0f;
        static constexpr float m_deceleration = 8.0f;
        static constexpr float m_maxSpeed = 600.0f;

        // Size parameters
        static constexpr float m_baseRadius = 20.0f;
        static constexpr float m_growthFactor = 1.5f;

        // Growth points per size - Reduced for harder difficulty
        static constexpr float m_tinyFishGrowth = 3.0f;    // Reduced from 5.0f
        static constexpr float m_smallFishGrowth = 6.0f;   // Reduced from 10.0f
        static constexpr float m_mediumFishGrowth = 12.0f; // Reduced from 20.0f

        // Window bounds
        sf::Vector2u m_windowBounds;

        // Statistics
        int m_totalFishEaten;
        int m_damageTaken;

        // Visual effects
        struct VisualEffect
        {
            float scale;
            float rotation;
            sf::Color color;
            sf::Time duration;
        };
        std::vector<VisualEffect> m_activeEffects;

        // Eat animation
        float m_eatAnimationScale;
        static constexpr float m_eatAnimationSpeed = 10.0f;

        // Damage flash
        sf::Color m_damageFlashColor;
        float m_damageFlashIntensity;
    };
}