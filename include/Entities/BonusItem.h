#pragma once

#include "Entity.h"
#include "SpriteComponent.h"
#include "Utils/SpriteDrawable.h"
#include <random>

namespace FishGame
{
    class SpriteManager;
    // Bonus item types
    enum class BonusType
    {
        Starfish,
        PearlOyster,
        PowerUp
    };

    // Base class for all bonus items
    class BonusItem : public Entity
    {
    public:
        BonusItem(BonusType type, int points);
        virtual ~BonusItem() = default;

        // Entity interface
        EntityType getType() const override { return EntityType::PowerUp; }
        sf::FloatRect getBounds() const override;

        // Bonus item interface
        virtual BonusType getBonusType() const { return m_bonusType; }
        virtual int getPoints() const { return m_points; }
        virtual void onCollect();

        // Lifetime management
        void setLifetime(sf::Time lifetime) { m_lifetime = lifetime; }
        bool hasExpired() const { return m_lifetimeElapsed >= m_lifetime; }

        // Helper for derived update methods
        bool updateLifetime(sf::Time deltaTime);
        float computeBobbingOffset(float freqMul = 1.0f, float ampMul = 1.0f) const;

    protected:
        BonusType m_bonusType;
        int m_points;
        sf::Time m_lifetime;
        sf::Time m_lifetimeElapsed;

        // Visual effects
        float m_bobAmplitude;
        float m_bobFrequency;

    public:
        // Made public for spawner access
        float m_baseY;
    };

    // Starfish bonus item - fixed points
    class Starfish : public BonusItem, public AutoSpriteDrawable<Starfish>
    {
    public:
        Starfish();
        ~Starfish() override = default;

        // Setup sprite using the global SpriteManager
        void initializeSprite(SpriteManager& spriteManager);

        void update(sf::Time deltaTime) override;

    private:
        float m_rotation;

        static constexpr int m_starfishPoints = 25;
        static constexpr float m_rotationSpeed = 30.0f;
    };

    // Pearl Oyster - opens periodically with white/black pearls
    class PearlOyster : public BonusItem
    {
    public:
        PearlOyster();
        ~PearlOyster() override = default;

        void update(sf::Time deltaTime) override;
        void onCollect() override;

        bool isOpen() const { return m_isOpen; }
        bool hasBlackPearl() const { return m_hasBlackPearl; }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void updateOpenState(sf::Time deltaTime);
        void initializeSprite(SpriteManager& spriteManager);

    protected:
        sf::Sprite m_pearlSprite;
        const sf::Texture* m_openTexture{nullptr};
        const sf::Texture* m_closedTexture{nullptr};
        const sf::Texture* m_whitePearlTexture{nullptr};
        const sf::Texture* m_blackPearlTexture{nullptr};

        bool m_isOpen;
        bool m_hasBlackPearl;

        sf::Time m_stateTimer;
        sf::Time m_openDuration;
        sf::Time m_closedDuration;

        static constexpr int m_whitePearlPoints = 100;
        static constexpr int m_blackPearlPoints = 500;
        static constexpr float m_maxOpenAngle = 45.0f;
        static constexpr float m_blackPearlChance = 0.05f;

        // Random number generation
        static std::mt19937 s_randomEngine;
        static std::uniform_real_distribution<float> s_pearlChance;
    };
}