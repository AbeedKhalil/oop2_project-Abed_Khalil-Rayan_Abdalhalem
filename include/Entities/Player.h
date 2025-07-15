#pragma once

#include "GameConstants.h"
#include "Fish.h"
#include "SpriteManager.h"
#include "Animator.h"
#include <memory>
#include <string>
#include "GrowthMeter.h"
#include "FrenzySystem.h"
#include "IPowerUpManager.h"
#include "IScoreSystem.h"
#include "SoundPlayer.h"

namespace FishGame
{
    class PlayerInput;
    class PlayerGrowth;
    class PlayerVisual;

    class Player : public Entity
    {
    public:
        struct VisualEffect
        {
            float scale = 1.f;
            float rotation = 0.f;
            sf::Color color = sf::Color::White;
            sf::Time duration = sf::Time::Zero;
        };
        Player();
        ~Player() override;

        // Entity interface implementation
        void update(sf::Time deltaTime) override;
        sf::FloatRect getBounds() const override;
        EntityType getType() const override { return EntityType::Player; }

        // Initialize with game systems
        void initializeSystems(GrowthMeter* growthMeter, FrenzySystem* frenzySystem,
            IPowerUpManager* powerUpManager, IScoreSystem* scoreSystem);

        // Player-specific methods
        void handleInput();
        sf::Vector2f getTargetPosition() const { return m_targetPosition; }

        // Sprite initialization
        void initializeSprite(SpriteManager& spriteManager);

        TextureID getTextureID() const;

        // Growth and size
        void grow(int scoreValue);
        void addPoints(int points);
        void resetSize();
        void fullReset();
        void checkStageAdvancement();

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

        void setSoundPlayer(SoundPlayer* player) { m_soundPlayer = player; }

        // Size information
        bool isAtMaxSize() const { return m_currentStage >= Constants::MAX_STAGES; }
        void setWindowBounds(const sf::Vector2u& windowSize);

        // Accessors used by helper classes
        bool areControlsReversed() const { return m_controlsReversed; }
        sf::Time getPoisonColorTimer() const { return m_poisonColorTimer; }
        void setPoisonColorTimer(sf::Time t) { m_poisonColorTimer = t; }

        sf::Time getSpeedBoostTimer() const { return m_speedBoostTimer; }
        float getSpeedMultiplier() const { return m_speedMultiplier; }

        int getCurrentStage() const { return m_currentStage; }
        void setCurrentStage(int stage) { m_currentStage = stage; }

        float getGrowthProgress() const { return m_growthProgress; }
        void setGrowthProgress(float value) { m_growthProgress = value; }
        void addGrowthProgress(float delta) { m_growthProgress += delta; }

        int getScore() const { return m_score; }
        void setScore(int score) { m_score = score; }

        void setPoints(int points) { m_points = points; }
        void incrementPoints(int amount) { m_points += amount; }

        SoundPlayer* getSoundPlayer() const { return m_soundPlayer; }
        GrowthMeter* getGrowthMeter() const { return m_growthMeter; }
        SpriteManager* getSpriteManager() const { return m_spriteManager; }
        Animator* getAnimator() const { return m_animator.get(); }
        std::vector<VisualEffect>& getActiveEffects() { return m_activeEffects; }
        const std::vector<VisualEffect>& getActiveEffects() const { return m_activeEffects; }

        sf::Time getEatAnimationTimer() const { return m_eatAnimationTimer; }
        void setEatAnimationTimer(sf::Time t) { m_eatAnimationTimer = t; }
        float getEatAnimationScale() const { return m_eatAnimationScale; }
        void setEatAnimationScale(float s) { m_eatAnimationScale = s; }
        sf::Time getTurnAnimationTimer() const { return m_turnAnimationTimer; }
        void setTurnAnimationTimer(sf::Time t) { m_turnAnimationTimer = t; }
        float getDamageFlashIntensity() const { return m_damageFlashIntensity; }
        void setDamageFlashIntensity(float f) { m_damageFlashIntensity = f; }
        sf::Color getDamageFlashColor() const { return m_damageFlashColor; }
        void setDamageFlashColor(sf::Color c) { m_damageFlashColor = c; }
        const std::string& getCurrentAnimation() const { return m_currentAnimation; }
        void setCurrentAnimation(const std::string& anim) { m_currentAnimation = anim; }
        bool isFacingRight() const { return m_facingRight; }
        sf::Time getInvulnerabilityTimer() const { return m_invulnerabilityTimer; }

        static constexpr float baseSpeed() { return m_baseSpeed; }
        static constexpr float baseRadius() { return m_baseRadius; }
        static constexpr float growthFactor() { return m_growthFactor; }
        static constexpr float tinyFishGrowth() { return m_tinyFishGrowth; }
        static constexpr float smallFishGrowth() { return m_smallFishGrowth; }
        static constexpr float mediumFishGrowth() { return m_mediumFishGrowth; }
        static constexpr float eatAnimationSpeed() { return m_eatAnimationSpeed; }
        static sf::Time eatAnimationDuration() { return m_eatAnimationDuration; }
        static sf::Time turnAnimationDuration() { return m_turnAnimationDuration; }

        // Statistics tracking

        // Visual effects
        void triggerEatEffect();
        void triggerDamageEffect();

        // Double dispatch handlers
        void onCollideWith(Entity& other, CollisionSystem& system) override;
        void onCollideWith(Fish& fish, CollisionSystem& system) override;
        void onCollideWith(Hazard& hazard, CollisionSystem& system) override;

    private:
        void updateVisualEffects(sf::Time deltaTime);

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void constrainToWindow();
        void updateInvulnerability(sf::Time deltaTime);

    private:
        int m_score;
        int m_currentStage;
        float m_growthProgress;

        // Auto-orientation
        bool m_autoOrient;
        static constexpr float m_orientationThreshold = 5.0f;

        // Points system
        int m_points;

        // Control state
        sf::Vector2f m_targetPosition;
        bool m_controlsReversed{ false };
        sf::Time m_poisonColorTimer{ sf::Time::Zero };

        // System references
        GrowthMeter* m_growthMeter;
        FrenzySystem* m_frenzySystem;
        IPowerUpManager* m_powerUpManager;
        IScoreSystem* m_scoreSystem;
        SpriteManager* m_spriteManager;
        SoundPlayer* m_soundPlayer{ nullptr };

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

        // Visual effects
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
        std::unique_ptr<PlayerInput> m_input;
        std::unique_ptr<PlayerGrowth> m_growth;
        std::unique_ptr<PlayerVisual> m_visual;
        bool m_facingRight{ false };
    };
}
