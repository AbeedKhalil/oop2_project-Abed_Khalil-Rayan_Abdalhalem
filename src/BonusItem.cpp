// BonusItem.cpp
#include "BonusItem.h"
#include <cmath>
#include <algorithm>

namespace FishGame
{
    // Static member initialization for PearlOyster
    std::mt19937 PearlOyster::s_randomEngine(std::random_device{}());
    std::uniform_real_distribution<float> PearlOyster::s_pearlChance(0.0f, 1.0f);

    // BonusItem implementation
    BonusItem::BonusItem(BonusType type, int points)
        : Entity()
        , m_bonusType(type)
        , m_points(points)
        , m_lifetime(sf::seconds(10.0f))
        , m_lifetimeElapsed(sf::Time::Zero)
        , m_bobAmplitude(10.0f)
        , m_bobFrequency(2.0f)
        , m_baseY(0.0f)
    {
    }

    sf::FloatRect BonusItem::getBounds() const
    {
        return sf::FloatRect(m_position.x - m_radius, m_position.y - m_radius,
            m_radius * 2.0f, m_radius * 2.0f);
    }

    // Starfish implementation
    Starfish::Starfish()
        : BonusItem(BonusType::Starfish, m_starfishPoints)
        , m_shape(20.0f, 5)
        , m_arms()
        , m_rotation(0.0f)
    {
        m_radius = 20.0f;

        // Setup main shape
        m_shape.setFillColor(sf::Color(255, 192, 203)); // Pink
        m_shape.setOutlineColor(sf::Color(220, 150, 170));
        m_shape.setOutlineThickness(2.0f);
        m_shape.setOrigin(m_radius, m_radius);

        // Create star arms using convex shapes
        m_arms.reserve(m_armCount);
        for (int i = 0; i < m_armCount; ++i)
        {
            sf::ConvexShape arm(4);
            float angle = (360.0f / m_armCount) * i;
            float radAngle = angle * 3.14159f / 180.0f;

            // Create arm points
            arm.setPoint(0, sf::Vector2f(0, 0));
            arm.setPoint(1, sf::Vector2f(std::cos(radAngle - 0.2f) * 15.0f,
                std::sin(radAngle - 0.2f) * 15.0f));
            arm.setPoint(2, sf::Vector2f(std::cos(radAngle) * 25.0f,
                std::sin(radAngle) * 25.0f));
            arm.setPoint(3, sf::Vector2f(std::cos(radAngle + 0.2f) * 15.0f,
                std::sin(radAngle + 0.2f) * 15.0f));

            arm.setFillColor(sf::Color(255, 182, 193));
            arm.setOutlineColor(sf::Color(220, 150, 170));
            arm.setOutlineThickness(1.0f);

            m_arms.push_back(std::move(arm));
        }
    }

    void Starfish::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        // Update lifetime
        m_lifetimeElapsed += deltaTime;
        if (hasExpired())
        {
            destroy();
            return;
        }

        // Rotation animation
        m_rotation += m_rotationSpeed * deltaTime.asSeconds();

        // Bobbing animation
        float bobOffset = std::sin(m_lifetimeElapsed.asSeconds() * m_bobFrequency) * m_bobAmplitude;
        m_position.y = m_baseY + bobOffset;

        // Update visual positions
        m_shape.setPosition(m_position);
        m_shape.setRotation(m_rotation);

        // Update arms
        std::for_each(m_arms.begin(), m_arms.end(),
            [this](sf::ConvexShape& arm) {
                arm.setPosition(m_position);
                arm.setRotation(m_rotation);
            });
    }

    void Starfish::onCollect()
    {
        // Visual/audio feedback would go here
        destroy();
    }

    void Starfish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Draw arms first
        std::for_each(m_arms.begin(), m_arms.end(),
            [&target, &states](const sf::ConvexShape& arm) {
                target.draw(arm, states);
            });

        // Draw center
        target.draw(m_shape, states);
    }

    // PearlOyster implementation
    PearlOyster::PearlOyster()
        : BonusItem(BonusType::PearlOyster, 0)
        , m_topShell(4)
        , m_bottomShell(4)
        , m_pearl(8.0f)
        , m_isOpen(false)
        , m_hasBlackPearl(false)
        , m_openAngle(0.0f)
        , m_stateTimer(sf::Time::Zero)
        , m_openDuration(sf::seconds(3.0f))
        , m_closedDuration(sf::seconds(5.0f))
    {
        m_radius = 30.0f;
        m_lifetime = sf::seconds(30.0f); // Longer lifetime for oysters

        // Determine pearl type
        m_hasBlackPearl = s_pearlChance(s_randomEngine) < m_blackPearlChance;
        m_points = m_hasBlackPearl ? m_blackPearlPoints : m_whitePearlPoints;

        // Setup shells
        // Top shell (trapezoid shape)
        m_topShell.setPoint(0, sf::Vector2f(-20, 0));
        m_topShell.setPoint(1, sf::Vector2f(20, 0));
        m_topShell.setPoint(2, sf::Vector2f(15, -25));
        m_topShell.setPoint(3, sf::Vector2f(-15, -25));
        m_topShell.setFillColor(sf::Color(169, 169, 169)); // Gray
        m_topShell.setOutlineColor(sf::Color(105, 105, 105));
        m_topShell.setOutlineThickness(2.0f);

        // Bottom shell
        m_bottomShell.setPoint(0, sf::Vector2f(-20, 0));
        m_bottomShell.setPoint(1, sf::Vector2f(20, 0));
        m_bottomShell.setPoint(2, sf::Vector2f(15, 25));
        m_bottomShell.setPoint(3, sf::Vector2f(-15, 25));
        m_bottomShell.setFillColor(sf::Color(169, 169, 169));
        m_bottomShell.setOutlineColor(sf::Color(105, 105, 105));
        m_bottomShell.setOutlineThickness(2.0f);

        // Setup pearl
        m_pearl.setFillColor(m_hasBlackPearl ? sf::Color(50, 50, 50) : sf::Color(250, 250, 250));
        m_pearl.setOutlineColor(m_hasBlackPearl ? sf::Color::Black : sf::Color(200, 200, 200));
        m_pearl.setOutlineThickness(1.0f);
        m_pearl.setOrigin(8.0f, 8.0f);
    }

    void PearlOyster::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        // Update lifetime
        m_lifetimeElapsed += deltaTime;
        if (hasExpired())
        {
            destroy();
            return;
        }

        // Update open/close state
        updateOpenState(deltaTime);

        // Bobbing animation
        float bobOffset = std::sin(m_lifetimeElapsed.asSeconds() * m_bobFrequency * 0.5f) * m_bobAmplitude * 0.5f;
        m_position.y = m_baseY + bobOffset;

        // Update positions
        m_topShell.setPosition(m_position);
        m_bottomShell.setPosition(m_position);
        m_pearl.setPosition(m_position);

        // Apply opening animation
        m_topShell.setRotation(-m_openAngle);
        m_bottomShell.setRotation(m_openAngle);
    }

    void PearlOyster::onCollect()
    {
        if (m_isOpen)
        {
            // Can only collect when open
            destroy();
        }
    }

    void PearlOyster::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_bottomShell, states);

        if (m_isOpen)
        {
            target.draw(m_pearl, states);
        }

        target.draw(m_topShell, states);
    }

    void PearlOyster::updateOpenState(sf::Time deltaTime)
    {
        m_stateTimer += deltaTime;

        if (m_isOpen)
        {
            // Opening animation
            if (m_openAngle < m_maxOpenAngle)
            {
                m_openAngle = std::min(m_openAngle + 90.0f * deltaTime.asSeconds(), m_maxOpenAngle);
            }

            // Check if should close
            if (m_stateTimer >= m_openDuration)
            {
                m_isOpen = false;
                m_stateTimer = sf::Time::Zero;
            }
        }
        else
        {
            // Closing animation
            if (m_openAngle > 0.0f)
            {
                m_openAngle = std::max(m_openAngle - 90.0f * deltaTime.asSeconds(), 0.0f);
            }

            // Check if should open
            if (m_stateTimer >= m_closedDuration)
            {
                m_isOpen = true;
                m_stateTimer = sf::Time::Zero;
            }
        }
    }
}