#pragma once

#include "Entity.h"
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
        friend class CollisionSystem;
    public:
        BonusItem(BonusType type, int points);
        virtual ~BonusItem() = default;

        // Entity interface
        EntityType getType() const override { return EntityType::PowerUp; }
        sf::FloatRect getBounds() const override;

        // Bonus item interface
        virtual BonusType getBonusType() const { return m_bonusType; }
        virtual int getPoints() const { return m_points; }
        virtual void onCollect() = 0;

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
        // Made public for spawner access...
        float m_baseY;
        void onCollide(Player& player, CollisionSystem& system) override;
    };

    // Starfish bonus item - fixed points
    class Starfish : public BonusItem
    {
    public:
        Starfish();
        ~Starfish() override = default;

        // Setup sprite using the global SpriteManager
        void initializeSprite(SpriteManager& spriteManager);

        void update(sf::Time deltaTime) override;
        void onCollect() override;

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        sf::CircleShape m_shape;
        std::vector<sf::ConvexShape> m_arms;
        float m_rotation;

        static constexpr int m_starfishPoints = 25;
        static constexpr float m_rotationSpeed = 30.0f;
        static constexpr int m_armCount = 5;
    };
}
