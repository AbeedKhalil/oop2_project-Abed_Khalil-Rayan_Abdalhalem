#pragma once

#include "Entity.h"
#include <vector>
#include <random>
#include <memory>
#include "Utils/AnimatedSprite.h"

namespace FishGame
{
    class SpriteManager;
    enum class HazardType
    {
        Bomb,
        Jellyfish
    };

    // Base class for all hazards
    class Hazard : public Entity
    {
    public:
        Hazard(HazardType type, float damageAmount);
        virtual ~Hazard() = default;

        EntityType getType() const override { return EntityType::Hazard; }
        HazardType getHazardType() const { return m_hazardType; }
        float getDamageAmount() const { return m_damageAmount; }

        virtual void onContact(Entity& entity) = 0;

    protected:
        HazardType m_hazardType;
        float m_damageAmount;
        sf::Time m_activationDelay;
        bool m_isActivated;
    };

    // Bomb hazard - explodes on contact
    class Bomb : public Hazard
    {
    public:
        Bomb();
        ~Bomb() override = default;

        // Sprite setup
        void initializeSprite(SpriteManager& spriteManager);

        void update(sf::Time deltaTime) override;
        sf::FloatRect getBounds() const override;
        void onContact(Entity& entity) override;

        // Control the explosion sequence
        void trigger();
        bool isFinished() const;

        // Compatibility with old systems
        bool isExploding() const { return m_isExploding; }
        float getExplosionRadius() const { return m_explosionRadius; }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        enum class State { IdleBomb, Explode, Puffs, Smoke, Done };

        void advanceState();

        // Animation
        std::unique_ptr<class AnimatedSprite> m_sprite;
        State m_state;
        int m_puffLoops;

        bool m_isExploding;
        sf::Time m_stateTimer;
        float m_explosionRadius;

        static constexpr float m_baseRadius = 20.0f;
        static constexpr float m_maxExplosionRadius = 100.0f;
        static constexpr float m_explosionDuration = 0.4f; // 5 frames * 0.08s
        static constexpr int m_maxPuffLoops = 3;
        static constexpr float m_puffFrameTime = 0.1f;
    };

    // Jellyfish - stuns on contact
    class Jellyfish : public Hazard
    {
    public:
        Jellyfish();
        ~Jellyfish() override = default;

        void initializeSprite(SpriteManager& spriteManager);

        void update(sf::Time deltaTime) override;
        sf::FloatRect getBounds() const override;
        void onContact(Entity& entity) override;

        sf::Time getStunDuration() const { return m_stunDuration; }

        // Push collided entity slightly forward
        void pushEntity(Entity& entity) const;

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        sf::CircleShape m_bell;
        std::vector<sf::RectangleShape> m_tentacles;
        float m_floatAnimation;
        float m_tentacleWave;
        sf::Time m_stunDuration;

        static constexpr float m_stunEffectDuration = 1.0f;
        static constexpr int m_tentacleCount = 8;

        static constexpr float m_pushDistance = 15.0f;
        static constexpr float m_pushForce = 300.0f;

        // Sprite animation
        int m_frame;
        sf::Time m_frameTimer;
        static constexpr int m_frameCount = 17;
        static constexpr float m_frameTime = 0.1f; // seconds per frame
        static constexpr int m_frameHeight = 197;
        int m_frameWidth{ 0 };
        const sf::Texture* m_texture{ nullptr };
    };
}