#pragma once

#include "Entity.h"
#include "Animator.h"
#include <vector>
#include <memory>
#include <string>

namespace FishGame
{
    // Forward declarations
    class SpriteManager;
    enum class TextureID;
    template<typename T> class SpriteComponent;

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

        // New fleeing behavior methods
        void startFleeing();
        bool isFleeing() const { return m_isFleeing; }
        void updateFleeingBehavior(sf::Time deltaTime);

        void setFrozen(bool frozen);
        bool isFrozen() const { return m_isFrozen; }

        // New state management methods
        void setPoisoned(sf::Time duration);
        void setStunned(sf::Time duration);
        bool isPoisoned() const { return m_isPoisoned; }
        bool isStunned() const { return m_isStunned; }

        // Special fish type checking
        virtual bool isSpecialFish() const { return false; }
        virtual bool isBarracuda() const { return false; }

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

        // Sprite support methods
        void initializeSprite(SpriteManager& spriteManager);
        void initializeAnimation(SpriteManager& spriteManager);
        void updateVisualState();
        void playEatAnimation();

        // Base color for sprite
        void setBaseColor(const sf::Color& color);
        const sf::Color& getBaseColor() const { return m_baseColor; }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void updateMovement(sf::Time deltaTime);

        // Sprite-specific updates
        virtual void updateSpriteEffects(sf::Time deltaTime);

        // Get appropriate texture ID for this fish
        virtual TextureID getTextureID() const;

    protected:
        FishSize m_size;
        float m_speed;
        int m_pointValue;
        int m_currentLevel;
        sf::Vector2u m_windowBounds;

        // State members - these were missing from the error
        bool m_isPoisoned;
        bool m_isStunned;
        sf::Time m_poisonTimer;
        sf::Time m_stunTimer;
        sf::Vector2f m_originalVelocity;

        bool m_isFrozen;
        sf::Vector2f m_velocityBeforeFreeze;

        // State durations
        static constexpr float m_defaultPoisonDuration = 5.0f;
        static constexpr float m_defaultStunDuration = 1.0f;

        // Fleeing behavior
        bool m_isFleeing;
        float m_fleeSpeed;
        sf::Vector2f m_fleeDirection;

        static constexpr float m_fleeSpeedMultiplier = 3.0f;

        // Additional member for damage flash effect
        sf::Time m_damageFlashTimer;

        // Base color used when no special state is active
        sf::Color m_baseColor;

        // Animation support
        std::unique_ptr<Animator> m_animator;
        std::string m_currentAnimation;
        bool m_facingRight{ false };
        bool m_turning{ false };
        sf::Time m_turnTimer{ sf::Time::Zero };
        static constexpr float m_turnDuration = 0.45f;

        // Eat animation state
        bool m_eating{ false };
        sf::Time m_eatTimer{ sf::Time::Zero };
        static constexpr float m_eatDuration = 0.5f;
    };
}