#include "BonusItem.h"
#include "GameConstants.h"
#include "DrawHelpers.h"
#include "SpriteManager.h"
#include <cmath>
#include <algorithm>
#include <iterator>

namespace FishGame
{
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

    bool BonusItem::updateLifetime(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return false;

        m_lifetimeElapsed += deltaTime;
        if (hasExpired())
        {
            destroy();
            return false;
        }
        return true;
    }

    float BonusItem::computeBobbingOffset(float freqMul, float ampMul) const
    {
        return std::sin(m_lifetimeElapsed.asSeconds() * m_bobFrequency * freqMul) *
            m_bobAmplitude * ampMul;
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
        std::generate_n(std::back_inserter(m_arms), m_armCount,
            [this, i = 0]() mutable {
                sf::ConvexShape arm(4);
                float angle = (360.0f / static_cast<float>(m_armCount)) * static_cast<float>(i);
                float radAngle = angle * Constants::DEG_TO_RAD;

                // Create arm points
                arm.setPoint(0, sf::Vector2f(0, 0));
                arm.setPoint(1, sf::Vector2f(std::cos(radAngle - 0.2f) * 10.0f,
                    std::sin(radAngle - 0.2f) * 10.0f));
                arm.setPoint(2, sf::Vector2f(std::cos(radAngle) * 18.0f,
                    std::sin(radAngle) * 18.0f));
                arm.setPoint(3, sf::Vector2f(std::cos(radAngle + 0.2f) * 10.0f,
                    std::sin(radAngle + 0.2f) * 10.0f));

                arm.setFillColor(sf::Color(255, 182, 193));
                arm.setOutlineColor(sf::Color(220, 150, 170));
                arm.setOutlineThickness(1.0f);

                ++i;
                return arm;
            });
    }

    void Starfish::initializeSprite(SpriteManager& spriteManager)
    {
        auto sprite = spriteManager.createSpriteComponent(
            static_cast<Entity*>(this), TextureID::Starfish);
        if (sprite)
        {
            auto config = spriteManager.getSpriteConfig<Entity>(TextureID::Starfish);
            sprite->configure(config);
            setSpriteComponent(std::move(sprite));
        }
    }

    void Starfish::update(sf::Time deltaTime)
    {
        if (!updateLifetime(deltaTime))
            return;

        if (getSpriteComponent())
        {
            getSpriteComponent()->update(deltaTime);
        }

        // Rotation animation
        m_rotation += m_rotationSpeed * deltaTime.asSeconds();

        // Bobbing animation
        m_position.y = m_baseY + computeBobbingOffset();

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
        if (getSpriteComponent())
        {
            target.draw(*getSpriteComponent(), states);
        }
        else
        {
            // Draw arms first
            DrawUtils::drawContainer(m_arms, target, states);

            // Draw center
            target.draw(m_shape, states);
        }
    }
}
