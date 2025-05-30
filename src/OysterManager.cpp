// OysterManager.cpp
#include "OysterManager.h"
#include <algorithm>

namespace FishGame
{
    PermanentOyster::PermanentOyster()
        : PearlOyster()
        , m_recentlyCollected(false)
        , m_collectionCooldown(sf::Time::Zero)
    {
        // Permanent oysters never expire
        m_lifetime = sf::seconds(999999.0f);
    }

    void PermanentOyster::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        // Update collection cooldown
        if (m_recentlyCollected)
        {
            m_collectionCooldown -= deltaTime;
            if (m_collectionCooldown <= sf::Time::Zero)
            {
                m_recentlyCollected = false;

                // Regenerate pearl type
                m_hasBlackPearl = s_pearlChance(s_randomEngine) < m_blackPearlChance;
                m_points = m_hasBlackPearl ? m_blackPearlPoints : m_whitePearlPoints;

                // Update pearl color
                m_pearl.setFillColor(m_hasBlackPearl ? sf::Color(50, 50, 50) : sf::Color(250, 250, 250));
            }
        }

        // Update open/close state
        updateOpenState(deltaTime);

        // No bobbing for permanent oysters - they stay fixed
        // Skip the base class bobbing animation

        // Update positions
        m_topShell.setPosition(m_position);
        m_bottomShell.setPosition(m_position);
        m_pearl.setPosition(m_position);

        // Apply opening animation
        m_topShell.setRotation(-m_openAngle);
        m_bottomShell.setRotation(m_openAngle);
    }

    void PermanentOyster::onCollect()
    {
        if (m_isOpen && !m_recentlyCollected)
        {
            // Mark as collected but don't destroy
            m_recentlyCollected = true;
            m_collectionCooldown = sf::seconds(m_cooldownDuration);

            // Close immediately after collection
            m_isOpen = false;
            m_stateTimer = sf::Time::Zero;
        }
    }

    void PermanentOyster::reset()
    {
        m_recentlyCollected = false;
        m_collectionCooldown = sf::Time::Zero;
        m_isOpen = false;
        m_stateTimer = sf::Time::Zero;
        m_openAngle = 0.0f;

        // Regenerate pearl type
        m_hasBlackPearl = s_pearlChance(s_randomEngine) < m_blackPearlChance;
        m_points = m_hasBlackPearl ? m_blackPearlPoints : m_whitePearlPoints;
        m_pearl.setFillColor(m_hasBlackPearl ? sf::Color(50, 50, 50) : sf::Color(250, 250, 250));
    }
}