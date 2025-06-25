#pragma once

#include "Fish.h"
#include "CollisionDetector.h"
#include "GenericFish.h"
#include "Animator.h"
#include "GameConstants.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <memory>

namespace FishGame
{
    // Movement pattern strategies
    enum class MovementPattern
    {
        Linear,
        Sinusoidal,
        ZigZag,
        Aggressive
    };

    // Enhanced base fish with advanced movement
    class AdvancedFish : public Fish
    {
    public:
        AdvancedFish(FishSize size, float speed, int currentLevel, MovementPattern pattern);
        virtual ~AdvancedFish() = default;

        void update(sf::Time deltaTime) override;
        void setMovementPattern(MovementPattern pattern) { m_movementPattern = pattern; }

    protected:
        virtual void updateMovementPattern(sf::Time deltaTime);

    protected:
        MovementPattern m_movementPattern;
        float m_patternTimer;
        float m_baseY;  // For sinusoidal movement
        float m_amplitude;
        float m_frequency;
    };

    // Barracuda - Fast predator with hunting AI
    class Barracuda : public AdvancedFish
    {
    public:
        explicit Barracuda(int currentLevel = 1);
        ~Barracuda() override = default;

        EntityType getType() const override { return EntityType::LargeFish; }

        TextureID getTextureID() const override { return TextureID::Barracuda; }
        int getScorePoints() const override { return Constants::BARRACUDA_POINTS; }

        // Enhanced AI for aggressive hunting
        void updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
            const Entity* player, sf::Time deltaTime);

        void update(sf::Time deltaTime) override;
        void initializeSprite(SpriteManager& spriteManager);
        void playEatAnimation() override;
    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updateHuntingBehavior(const Entity* target, sf::Time /*deltaTime*/);

    private:
        const Entity* m_currentTarget;
        sf::Time m_huntTimer;
        float m_dashSpeed;
        bool m_isDashing;

        std::unique_ptr<Animator> m_animator;
        std::string m_currentAnimation;
        bool m_facingRight{ false };
        bool m_turning{ false };
        sf::Time m_turnTimer{ sf::Time::Zero };

        static constexpr float m_turnDuration = 0.45f;
        static constexpr float m_huntRange = 250.0f;
        static constexpr float m_dashMultiplier = 2.5f;
        static constexpr float m_dashDuration = 1.0f;
    };

    // Pufferfish - Inflates periodically
    class Pufferfish : public AdvancedFish
    {
    public:
        explicit Pufferfish(int currentLevel = 1);
        ~Pufferfish() override = default;

        EntityType getType() const override { return EntityType::MediumFish; }

        TextureID getTextureID() const override
        {
            return isInflated() ? TextureID::PufferfishInflated : TextureID::Pufferfish;
        }
        int getScorePoints() const override { return Constants::PUFFERFISH_POINTS; }

        void update(sf::Time deltaTime) override;
        void initializeSprite(SpriteManager& spriteManager);
        bool canEat(const Entity& other) const;

        // Inflation state
        bool isInflated() const { return m_isPuffed; }

        // Push mechanics
        void pushEntity(Entity& entity);
        bool canPushEntity(const Entity& entity) const;

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updateCycleState(sf::Time deltaTime);
        void transitionToInflated();
        void transitionToNormal();

        enum class PuffPhase { None, Inflating, Holding, Deflating };

    private:
        // State management
        bool m_isPuffed;
        sf::Time m_stateTimer;
        float m_inflationLevel;  // 0.0 = normal, 1.0 = fully inflated
        float m_normalRadius;

        // Visual elements
        std::vector<sf::CircleShape> m_spikes;

        // Push mechanics
        static constexpr float m_pushDistance = 10.0f;
        static constexpr float m_pushForce = 500.0f;

        // Timing constants
        static constexpr float m_normalStateDuration = 5.0f;  // 5 seconds normal
        static constexpr float m_puffedStateDuration = 5.0f;  // 5 seconds puffed
        static constexpr float m_inflationSpeed = 3.0f;
        static constexpr float m_deflationSpeed = 3.0f;
        static constexpr float m_inflatedRadiusMultiplier = 2.0f;
        static constexpr int m_spikeCount = 8;

        // Animation state
        bool m_isPuffing{ false };
        sf::Time m_puffTimer{ sf::Time::Zero };
        static constexpr float m_puffAnimDuration = 0.6f; // 6 frames * 0.1s
        PuffPhase m_puffPhase{ PuffPhase::None };
    };

    // PoisonFish - Can be eaten but reverses controls when consumed
    class PoisonFish : public AdvancedFish
    {
    public:
        explicit PoisonFish(int currentLevel = 1);
        ~PoisonFish() override = default;

        TextureID getTextureID() const override { return TextureID::PoisonFish; }

        EntityType getType() const override { return EntityType::SmallFish; }
        int getPointValue() const override { return m_poisonPoints; }
        int getScorePoints() const override { return m_poisonPoints; }

        void update(sf::Time deltaTime) override;

        // Get poison effect duration when eaten
        sf::Time getPoisonDuration() const { return m_poisonDuration; }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updatePoisonBubbles(sf::Time /*deltaTime*/);

    private:
        std::vector<sf::CircleShape> m_poisonBubbles;
        float m_wobbleAnimation;
        sf::Time m_poisonDuration;
        int m_poisonPoints; // Negative points for eating poison fish

        static constexpr float m_poisonEffectDuration = 5.0f;
        static constexpr int m_basePoisonPoints = -10; // Penalty for eating
        static constexpr int m_bubbleCount = 6;
    };

    // Angelfish - Gives bonus points and has erratic movement
    class Angelfish : public AdvancedFish
    {
    public:
        explicit Angelfish(int currentLevel = 1);
        ~Angelfish() override = default;

        EntityType getType() const override { return EntityType::SmallFish; }
        int getPointValue() const { return m_bonusPoints; }
        TextureID getTextureID() const override { return TextureID::Angelfish; }
        int getScorePoints() const override { return Constants::ANGELFISH_POINTS; }

        void update(sf::Time deltaTime) override;

        // Enhanced AI for evasive behavior
        void updateAI(const std::vector<std::unique_ptr<Entity>>& entities,
            const Entity* player, sf::Time deltaTime) override;

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updateErraticMovement(sf::Time deltaTime);
        void updateEvasiveMovement(const std::vector<std::unique_ptr<Entity>>& entities,
            const Entity* player);
        sf::Vector2f calculateEscapeVector(const std::vector<const Entity*>& threats);

    private:
        int m_bonusPoints;
        float m_colorShift;
        std::vector<sf::CircleShape> m_fins;
        sf::Time m_directionChangeTimer;

        // Enhanced AI properties
        const Entity* m_currentThreat;
        bool m_isEvading;
        sf::Time m_evasionTimer;

        // Movement parameters
        static constexpr float m_baseSpeed = 280.0f;  // Increased from 200.0f
        static constexpr float m_evadeSpeed = 400.0f; // Fast escape speed
        static constexpr float m_directionChangeInterval = 0.3f; // Faster reactions
        static constexpr float m_threatDetectionRange = 150.0f;
        static constexpr float m_panicRange = 80.0f;
        static constexpr int m_baseBonus = 50;
    };

    // Template for creating schools of fish
    template<typename FishType>
    class SchoolMember : public FishType
    {
        static_assert(std::is_base_of_v<Fish, FishType>,
            "SchoolMember can only be used with Fish types");

    public:
        // Single constructor that works for all fish types
        explicit SchoolMember(int currentLevel = 1)
            : FishType(currentLevel)
            , m_schoolId(-1)
            , m_neighborDistance(80.0f)
            , m_separationDistance(30.0f)
        {
        }

        void setSchoolId(int id) { m_schoolId = id; }
        int getSchoolId() const { return m_schoolId; }

        // Flocking behavior update
        void updateSchooling(const std::vector<SchoolMember*>& schoolmates, sf::Time deltaTime)
        {
            if (schoolmates.empty()) return;

            sf::Vector2f separation(0, 0);
            sf::Vector2f alignment(0, 0);
            sf::Vector2f cohesion(0, 0);

            int separationCount = 0;
            int neighborCount = 0;

            // Apply flocking rules
            std::for_each(schoolmates.begin(), schoolmates.end(),
                [this, &separation, &alignment, &cohesion,
                &separationCount, &neighborCount](const SchoolMember* mate)
                {
                    if (mate == this || !mate->isAlive()) return;

                    float distance = CollisionDetector::getDistance(
                        this->m_position, mate->getPosition());

                    // Separation
                    if (distance < m_separationDistance && distance > 0)
                    {
                        sf::Vector2f diff = this->m_position - mate->getPosition();
                        diff /= distance;  // Normalize
                        separation += diff;
                        separationCount++;
                    }

                    // Alignment and Cohesion
                    if (distance < m_neighborDistance)
                    {
                        alignment += mate->getVelocity();
                        cohesion += mate->getPosition();
                        neighborCount++;
                    }
                });

            // Apply forces
            sf::Vector2f steer(0, 0);

            if (separationCount > 0)
            {
                separation /= static_cast<float>(separationCount);
                steer += separation * 1.5f;  // Weight separation higher
            }

            if (neighborCount > 0)
            {
                // Alignment
                alignment /= static_cast<float>(neighborCount);
                alignment = normalizeVector(alignment) * this->m_speed;
                steer += (alignment - this->m_velocity) * 0.5f;

                // Cohesion
                cohesion /= static_cast<float>(neighborCount);
                sf::Vector2f seekPos = cohesion - this->m_position;
                seekPos = normalizeVector(seekPos) * this->m_speed;
                steer += (seekPos - this->m_velocity) * 0.3f;
            }

            // Apply steering force
            this->m_velocity += steer * deltaTime.asSeconds();

            // Limit speed
            float currentSpeed = std::sqrt(this->m_velocity.x * this->m_velocity.x +
                this->m_velocity.y * this->m_velocity.y);
            if (currentSpeed > this->m_speed)
            {
                this->m_velocity = (this->m_velocity / currentSpeed) * this->m_speed;
            }
        }

    private:
        sf::Vector2f normalizeVector(const sf::Vector2f& vec)
        {
            float length = std::sqrt(vec.x * vec.x + vec.y * vec.y);
            if (length > 0)
                return vec / length;
            return vec;
        }

    private:
        int m_schoolId;
        float m_neighborDistance;
        float m_separationDistance;
    };
}
