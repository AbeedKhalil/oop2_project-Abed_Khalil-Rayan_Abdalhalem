#include "ExtendedPowerUps.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include "Utils/DrawHelpers.h"
#include "SpriteManager.h"

namespace FishGame
{
    // FreezePowerUp implementation
    FreezePowerUp::FreezePowerUp()
        : PowerUp(PowerUpType::Freeze, sf::seconds(m_freezeDuration))
        , m_icon()
        , m_iceShards()
    {
        m_icon.setString("*");
        m_icon.setCharacterSize(32);
        m_icon.setFillColor(sf::Color::Cyan);

        sf::FloatRect bounds = m_icon.getLocalBounds();
        m_icon.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);

        // Create ice shard effects
        m_iceShards.reserve(6);
        std::generate_n(std::back_inserter(m_iceShards), 6, []
            {
                sf::RectangleShape shard(sf::Vector2f(3.0f, 15.0f));
                shard.setFillColor(sf::Color(200, 240, 255));
                shard.setOrigin(1.5f, 7.5f);
                return shard;
            });

        m_aura.setOutlineColor(getAuraColor());
    }

    void FreezePowerUp::update(sf::Time deltaTime)
    {
        // DO NOT call base class update - implement everything here
        if (!updateLifetime(deltaTime))
            return;

        // Update animations
        m_pulseAnimation += deltaTime.asSeconds() * 2.0f;
        float pulse = 1.0f + 0.2f * std::sin(m_pulseAnimation);

        // Bobbing animation
        m_position.y = m_baseY + computeBobbingOffset();

        // Update visual positions
        m_iconBackground.setPosition(m_position);
        m_iconBackground.setScale(pulse, pulse);
        m_aura.setPosition(m_position);
        m_icon.setPosition(m_position);

        // Update ice shards
        for (size_t i = 0; i < m_iceShards.size(); ++i)
        {
            float angle = (60.0f * i + m_pulseAnimation * 30.0f) * Constants::DEG_TO_RAD;
            float radius = 20.0f + 5.0f * std::sin(m_pulseAnimation);

            sf::Vector2f shardPos(
                m_position.x + std::cos(angle) * radius,
                m_position.y + std::sin(angle) * radius
            );

            m_iceShards[i].setPosition(shardPos);
            m_iceShards[i].setRotation(angle * Constants::RAD_TO_DEG);
        }

        // Update aura glow
        sf::Color auraColor = getAuraColor();
        auraColor.a = static_cast<sf::Uint8>(128 + 127 * std::sin(m_pulseAnimation * 0.5f));
        m_aura.setOutlineColor(auraColor);
    }

    void FreezePowerUp::onCollect()
    {
        destroy();
    }

    void FreezePowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_aura, states);
        target.draw(m_iconBackground, states);

        DrawUtils::drawContainer(m_iceShards, target, states);

        target.draw(m_icon, states);
    }

    // ExtraLifePowerUp implementation
    ExtraLifePowerUp::ExtraLifePowerUp()
        : PowerUp(PowerUpType::ExtraLife, sf::Time::Zero)
        , m_heart(10.0f, 8)
        , m_heartbeatAnimation(0.0f)
    {
        m_heart.setFillColor(sf::Color(255, 100, 100));
        m_heart.setOutlineColor(sf::Color(200, 50, 50));
        m_heart.setOutlineThickness(2.0f);
        m_heart.setOrigin(10.0f, 10.0f);

        m_aura.setOutlineColor(getAuraColor());
    }

    void ExtraLifePowerUp::update(sf::Time deltaTime)
    {
        // DO NOT call base class update - implement everything here
        if (!updateLifetime(deltaTime))
            return;

        if (getRenderMode() == RenderMode::Sprite && getSpriteComponent())
            getSpriteComponent()->update(deltaTime);

        // Update animations
        m_heartbeatAnimation += deltaTime.asSeconds() * m_heartbeatSpeed;
        float heartbeat = 1.0f + 0.2f * std::sin(m_heartbeatAnimation);
        m_pulseAnimation += deltaTime.asSeconds() * 3.0f;

        // Bobbing animation
        m_position.y = m_baseY + computeBobbingOffset();

        // Update positions
        m_iconBackground.setPosition(m_position);
        m_aura.setPosition(m_position);
        m_heart.setPosition(m_position);
        m_heart.setScale(heartbeat, heartbeat);

        // Update aura
        sf::Color auraColor = getAuraColor();
        auraColor.a = static_cast<sf::Uint8>(128 + 127 * std::sin(m_pulseAnimation * 0.5f));
        m_aura.setOutlineColor(auraColor);
    }

    void ExtraLifePowerUp::onCollect()
    {
        destroy();
    }

    void ExtraLifePowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (getRenderMode() == RenderMode::Sprite && getSpriteComponent())
        {
            target.draw(*getSpriteComponent(), states);
        }
        else
        {
            target.draw(m_aura, states);
            target.draw(m_iconBackground, states);
            target.draw(m_heart, states);
        }
    }

    void ExtraLifePowerUp::initializeSprite(SpriteManager& spriteManager)
    {
        auto sprite = spriteManager.createSpriteComponent(
            static_cast<Entity*>(this), TextureID::PowerUpExtraLife);
        if (sprite)
        {
            auto config = spriteManager.getSpriteConfig<Entity>(TextureID::PowerUpExtraLife);
            sprite->configure(config);
            setSpriteComponent(std::move(sprite));
            setRenderMode(RenderMode::Sprite);
        }
    }

    // SpeedBoostPowerUp implementation
    SpeedBoostPowerUp::SpeedBoostPowerUp()
        : PowerUp(PowerUpType::SpeedBoost, sf::seconds(m_boostDuration))
        , m_speedLines()
        , m_lineAnimation(0.0f)
    {
        // Create speed line effects
        m_speedLines.reserve(4);
        std::generate_n(std::back_inserter(m_speedLines), 4, []
            {
                sf::ConvexShape line(4);
                line.setPoint(0, sf::Vector2f(0, -2));
                line.setPoint(1, sf::Vector2f(15, -1));
                line.setPoint(2, sf::Vector2f(15, 1));
                line.setPoint(3, sf::Vector2f(0, 2));
                line.setFillColor(sf::Color(0, 255, 255, 150));
                return line;
            });

        m_aura.setOutlineColor(getAuraColor());
    }

    void SpeedBoostPowerUp::update(sf::Time deltaTime)
    {
        // DO NOT call base class update - implement everything here
        if (!updateLifetime(deltaTime))
            return;

        if (getRenderMode() == RenderMode::Sprite && getSpriteComponent())
            getSpriteComponent()->update(deltaTime);

        // Update animations
        m_lineAnimation += deltaTime.asSeconds() * 5.0f;
        m_pulseAnimation += deltaTime.asSeconds() * 4.0f;
        float pulse = 1.0f + 0.15f * std::sin(m_pulseAnimation);

        // Bobbing animation
        m_position.y = m_baseY + computeBobbingOffset(1.5f);

        // Update positions
        m_iconBackground.setPosition(m_position);
        m_iconBackground.setScale(pulse, pulse);
        m_aura.setPosition(m_position);

        // Update speed lines
        for (size_t i = 0; i < m_speedLines.size(); ++i)
        {
            float angle = (90.0f * i) * Constants::DEG_TO_RAD;
            float offset = 10.0f + 10.0f * std::sin(m_lineAnimation + i);

            sf::Vector2f linePos(
                m_position.x + std::cos(angle) * offset,
                m_position.y + std::sin(angle) * offset
            );

            m_speedLines[i].setPosition(linePos);
            m_speedLines[i].setRotation(angle * Constants::RAD_TO_DEG);
        }

        // Update aura
        sf::Color auraColor = getAuraColor();
        auraColor.a = static_cast<sf::Uint8>(128 + 127 * std::sin(m_pulseAnimation * 0.5f));
        m_aura.setOutlineColor(auraColor);
    }

    void SpeedBoostPowerUp::onCollect()
    {
        destroy();
    }

    void SpeedBoostPowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (getRenderMode() == RenderMode::Sprite && getSpriteComponent())
        {
            target.draw(*getSpriteComponent(), states);
        }
        else
        {
            target.draw(m_aura, states);
            target.draw(m_iconBackground, states);

            DrawUtils::drawContainer(m_speedLines, target, states);
        }
    }

    void SpeedBoostPowerUp::initializeSprite(SpriteManager& spriteManager)
    {
        auto sprite = spriteManager.createSpriteComponent(
            static_cast<Entity*>(this), TextureID::PowerUpSpeedBoost);
        if (sprite)
        {
            auto config = spriteManager.getSpriteConfig<Entity>(TextureID::PowerUpSpeedBoost);
            sprite->configure(config);
            setSpriteComponent(std::move(sprite));
            setRenderMode(RenderMode::Sprite);
        }
    }

    // AddTimePowerUp implementation
    AddTimePowerUp::AddTimePowerUp()
        : PowerUp(PowerUpType::AddTime, sf::Time::Zero)
    {
    }

    void AddTimePowerUp::update(sf::Time deltaTime)
    {
        if (!updateLifetime(deltaTime))
            return;

        if (getRenderMode() == RenderMode::Sprite && getSpriteComponent())
            getSpriteComponent()->update(deltaTime);

        m_position.y = m_baseY + computeBobbingOffset();
        if (getRenderMode() == RenderMode::Sprite && getSpriteComponent())
            getSpriteComponent()->syncWithOwner();
    }

    void AddTimePowerUp::onCollect()
    {
        destroy();
    }

    void AddTimePowerUp::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (getRenderMode() == RenderMode::Sprite && getSpriteComponent())
        {
            target.draw(*getSpriteComponent(), states);
        }
    }

    void AddTimePowerUp::initializeSprite(SpriteManager& spriteManager)
    {
        auto sprite = spriteManager.createSpriteComponent(
            static_cast<Entity*>(this), TextureID::PowerUpAddTime);
        if (sprite)
        {
            auto config = spriteManager.getSpriteConfig<Entity>(TextureID::PowerUpAddTime);
            sprite->configure(config);
            setSpriteComponent(std::move(sprite));
            setRenderMode(RenderMode::Sprite);
        }
    }
}
