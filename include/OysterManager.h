// OysterManager.h
#pragma once

#include "BonusItem.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <array>
#include <algorithm>

namespace FishGame
{
    // Enhanced PearlOyster that can damage player when closed
    class PermanentOyster : public PearlOyster
    {
    public:
        PermanentOyster();
        ~PermanentOyster() override = default;

        void update(sf::Time deltaTime) override;
        void onCollect() override;

        // New methods for permanent oysters
        void reset();
        bool canDamagePlayer() const { return !m_isOpen && !m_recentlyCollected; }
        bool canBeEaten() const { return m_isOpen && !m_recentlyCollected; }

        // Growth points when eaten
        int getGrowthPoints() const { return m_hasBlackPearl ? 30 : 15; }
        bool hasBlackPearl() const { return m_hasBlackPearl; }

    private:
        bool m_recentlyCollected;
        sf::Time m_collectionCooldown;
        static constexpr float m_cooldownDuration = 5.0f;
    };

    // Template-based oyster management system
    template<size_t OysterCount = 3>
    class OysterManager
    {
    public:
        explicit OysterManager(const sf::Vector2u& windowSize)
            : m_windowSize(windowSize)
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
    };

    // Type alias for convenience
    using FixedOysterManager = OysterManager<3>;
}