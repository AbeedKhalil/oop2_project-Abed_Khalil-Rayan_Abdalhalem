#pragma once

#include "GameConstants.h"
#include "Fish.h"
#include "SpriteManager.h"
#include "Animator.h"
#include <memory>
#include <string>
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
        sf::Vector2f getTargetPosition() const { return m_targetPosition; }
        bool isUsingMouseControl() const { return m_useMouseControl; }

        // Sprite initialization
        void initializeSprite(SpriteManager& spriteManager);

        TextureID getTextureID() const;

        // Growth and size
        void grow(int scoreValue);
        void addPoints(int points);
        void resetSize();
        void fullReset();
        int getCurrentStage() const { return m_currentStage; }
        int getScore() const { return m_score; }
        float getGrowthProgress() const { return m_growthProgress; }

        void enableMouseControl(bool enable);
        void setMousePosition(const sf::Vector2f& screenPos);
        bool isMouseControlActive() const { return m_mouseControlActive; }
        void setAutoOrient(bool enable) { m_autoOrient = enable; }

        // Points tracking
        int getPoints() const { return m_points; }

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

        // Poison effect
        void applyPoisonEffect(sf::Time duration);
        void setControlsReversed(bool reversed) { m_controlsReversed = reversed; }
        bool areControlsReversed() const { return m_controlsReversed; }

        // Size information
        bool isAtMaxSize() const { return m_currentStage >= Constants::MAX_STAGES; }
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
        void checkStageAdvancement();

    private:
        int m_score;
        int m_currentStage;
        float m_growthProgress;

        // Mouse control enhancements
        bool m_mouseControlActive;
        static constexpr float m_mouseDeadzone = 2.0f;
        static constexpr float m_mouseSmoothingFactor = 0.15f;

        // Auto-orientation
        bool m_autoOrient;
        static constexpr float m_orientationThreshold = 5.0f;

        // Points system
        int m_points;

        // Control state
        bool m_useMouseControl;
        sf::Vector2f m_targetPosition;
        bool m_controlsReversed{ false };
        sf::Time m_poisonColorTimer{ sf::Time::Zero };

        // System references
        GrowthMeter* m_growthMeter;
        FrenzySystem* m_frenzySystem;
        PowerUpManager* m_powerUpManager;
        ScoreSystem* m_scoreSystem;
        SpriteManager* m_spriteManager;

        // Invulnerability and damage
        sf::Time m_invulnerabilityTimer;
        sf::Time m_damageCooldown;
        static const sf::Time m_invulnerabilityDuration;
        static const sf::Time m_damageCooldownDuration;

        // Power-up effects
        float m_speedMultiplier;
        sf::Time m_speedBoostTimer;     

        // Movement parameters
        static constexpr float m_baseSpeed = Constants::PLAYER_BASE_SPEED;
        static constexpr float m_acceleration = Constants::PLAYER_ACCELERATION;
        static constexpr float m_deceleration = Constants::PLAYER_DECELERATION;
        static constexpr float m_maxSpeed = Constants::PLAYER_MAX_SPEED;

        // Size parameters
        static constexpr float m_baseRadius = Constants::PLAYER_BASE_RADIUS;
        static constexpr float m_growthFactor = Constants::PLAYER_GROWTH_FACTOR;

        // Growth values for visual growth
        static constexpr float m_tinyFishGrowth = 3.0f;
        static constexpr float m_smallFishGrowth = 6.0f;
        static constexpr float m_mediumFishGrowth = 12.0f;

        // Window bounds
        sf::Vector2u m_windowBounds;

        // Statistics
        int m_totalFishEaten;
        int m_damageTaken;

        // Visual effects
        struct VisualEffect
        {
            float scale = 1.f;
            float rotation = 0.f;
            sf::Color color = sf::Color::White;
            sf::Time duration = sf::Time::Zero;
        };
        std::vector<VisualEffect> m_activeEffects;

        // Eat animation
        float m_eatAnimationScale;
        static constexpr float m_eatAnimationSpeed = 10.0f;
        sf::Time m_eatAnimationTimer;
        static const sf::Time m_eatAnimationDuration;

        // Turn animation
        sf::Time m_turnAnimationTimer;
        static const sf::Time m_turnAnimationDuration;

        // Damage flash
        sf::Color m_damageFlashColor;
        float m_damageFlashIntensity;

        // Animation
        std::unique_ptr<Animator> m_animator;
        std::string m_currentAnimation;
        bool m_facingRight{ false };
    };
}