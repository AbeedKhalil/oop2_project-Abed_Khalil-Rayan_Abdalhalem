#pragma once

#include "BonusItem.h"
#include "SpriteManager.h"
#include <array>
#include <algorithm>
#include <random>

namespace FishGame
{
    // Oyster using sprite-based animation
    class PermanentOyster : public BonusItem
    {
    public:
        PermanentOyster();
        ~PermanentOyster() override = default;

        void update(sf::Time deltaTime) override;
        void onCollect() override;

        // Sprite setup
        void initializeSprites(SpriteManager& spriteManager);

        // New methods for permanent oysters
        void reset();
        // Damage any entity that remains when the oyster is closing
        bool canDamagePlayer() const { return m_state == State::Closing; }
        bool canBeEaten() const { return m_state == State::Open && !m_recentlyCollected; }

        // Growth points when eaten
        int getGrowthPoints() const { return m_hasBlackPearl ? 30 : 15; }
        bool hasBlackPearl() const { return m_hasBlackPearl; }
        bool hasPearl() const { return m_hasPearlSprite; }
        bool isOpen() const { return m_isOpen; }

    private:
        enum class State { Closed, Opening, Open, Closing };

        bool m_recentlyCollected;
        sf::Time m_collectionCooldown;
        static constexpr float m_cooldownDuration = 5.0f;

        bool m_isOpen{ false };
        bool m_hasBlackPearl{ false };
        float m_openAngle{ 0.f };
        sf::Time m_stateTimer{ sf::Time::Zero };
        sf::Time m_openDuration{ sf::seconds(2.0f) };
        sf::Time m_closedDuration{ sf::seconds(3.0f) };

        static constexpr int m_whitePearlPoints = 100;
        static constexpr int m_blackPearlPoints = 500;
        static constexpr float m_maxOpenAngle = 45.0f;
        static std::mt19937 s_randomEngine;
        static std::uniform_real_distribution<float> s_pearlChance;

        // Animation
        State m_state{ State::Closed };
        int m_frame{ 0 };
        sf::Time m_frameTimer{ sf::Time::Zero };
        static constexpr int m_frameCount = 5;
        static constexpr float m_frameTime = 0.15f; // seconds per frame
        static constexpr float m_closingFrameTime = 0.08f;

        // Sprites
        sf::Sprite m_sprite;
        sf::Sprite m_pearlSprite;
        const sf::Texture* m_oysterTexture{ nullptr };
        const sf::Texture* m_whitePearlTex{ nullptr };
        const sf::Texture* m_blackPearlTex{ nullptr };
        bool m_hasPearlSprite{ false };

        void updateAnimation(sf::Time dt);
        void updateSprite();
        void spawnPearl();

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };

    // Template-based oyster management system
    template<size_t OysterCount = 3>
    class OysterManager
    {
    public:
        OysterManager(const sf::Vector2u& windowSize, SpriteManager& spriteMgr)
            : m_windowSize(windowSize), m_spriteManager(&spriteMgr)
        {
            initializeOysters();
        }

        void update(sf::Time deltaTime)
        {
            std::for_each(m_oysters.begin(), m_oysters.end(),
                [deltaTime](auto& oyster) {
                    oyster->update(deltaTime);
                });
        }

        void draw(sf::RenderTarget& target) const
        {
            std::for_each(m_oysters.begin(), m_oysters.end(),
                [&target](const auto& oyster) {
                    target.draw(*oyster);
                });
        }

        // Template method for collision checking
        template<typename CollisionFunc>
        void checkCollisions(const Entity& entity, CollisionFunc&& onCollision)
        {
            std::for_each(m_oysters.begin(), m_oysters.end(),
                [&entity, &onCollision](auto& oyster) {
                    if (oyster->isAlive() && checkCollision(entity, *oyster))
                    {
                        onCollision(oyster.get());
                    }
                });
        }

        void resetAll()
        {
            std::for_each(m_oysters.begin(), m_oysters.end(),
                [](auto& oyster) {
                    oyster->reset();
                });
        }

    private:
        void initializeOysters()
        {
            // Fixed positions distributed across bottom of screen
            std::array<float, OysterCount> xPositions;
            float spacing = static_cast<float>(m_windowSize.x) / (OysterCount + 1);

            // Generate positions using STL algorithm
            std::generate(xPositions.begin(), xPositions.end(),
                [spacing, n = 1]() mutable {
                    return spacing * n++;
                });

            // Create oysters at fixed positions
            std::transform(xPositions.begin(), xPositions.end(), m_oysters.begin(),
                [this](float x) {
                    auto oyster = std::make_unique<PermanentOyster>();
                    oyster->setPosition(x, m_windowSize.y - 80.0f);
                    oyster->m_baseY = m_windowSize.y - 80.0f;
                    if (m_spriteManager)
                        oyster->initializeSprites(*m_spriteManager);
                    return oyster;
                });
        }

        static bool checkCollision(const Entity& a, const Entity& b)
        {
            sf::Vector2f diff = a.getPosition() - b.getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;
            float radiusSum = a.getRadius() + b.getRadius();
            return distSq < radiusSum * radiusSum;
        }

    private:
        sf::Vector2u m_windowSize;
        std::array<std::unique_ptr<PermanentOyster>, OysterCount> m_oysters;
        SpriteManager* m_spriteManager;
    };

    // Type alias for convenience
    using FixedOysterManager = OysterManager<3>;
}