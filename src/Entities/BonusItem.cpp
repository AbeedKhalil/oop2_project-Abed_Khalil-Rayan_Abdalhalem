#include "BonusItem.h"
#include "GameConstants.h"
#include "SpriteManager.h"
#include <cmath>
#include <algorithm>
#include <iterator>

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
        , m_rotation(0.0f)
    {
        m_radius = 20.0f;
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
            setRenderMode(RenderMode::Sprite);
        }
    }

    void PearlOyster::initializeSprite(SpriteManager& spriteManager)
    {
        auto sprite = spriteManager.createSpriteComponent(
            static_cast<Entity*>(this), TextureID::PearlOysterClosed);
        if (sprite)
        {
            auto config = spriteManager.getSpriteConfig<Entity>(TextureID::PearlOysterClosed);
            sprite->configure(config);
            setSpriteComponent(std::move(sprite));
            setRenderMode(RenderMode::Sprite);

            m_openTexture = &spriteManager.getTexture(TextureID::PearlOysterOpen);
            m_closedTexture = &spriteManager.getTexture(TextureID::PearlOysterClosed);
            m_whitePearlTexture = &spriteManager.getTexture(TextureID::WhitePearl);
            m_blackPearlTexture = &spriteManager.getTexture(TextureID::BlackPearl);

            m_pearlSprite.setTexture(*m_whitePearlTexture);
            m_pearlSprite.setOrigin(m_pearlSprite.getLocalBounds().width / 2.f,
                                    m_pearlSprite.getLocalBounds().height / 2.f);
        }
    }

    void Starfish::update(sf::Time deltaTime)
    {
        if (!updateLifetime(deltaTime))
            return;

        // Rotation animation
        m_rotation += m_rotationSpeed * deltaTime.asSeconds();

        if (getSpriteComponent())
        {
            getSpriteComponent()->update(deltaTime);
            getSpriteComponent()->setRotation(m_rotation);
        }

        // Bobbing animation
        m_position.y = m_baseY + computeBobbingOffset();

    }

    void Starfish::onCollect()
    {
        // Visual/audio feedback would go here
        destroy();
    }

    void Starfish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (getSpriteComponent())
            target.draw(*getSpriteComponent(), states);
    }

    // PearlOyster implementation
    PearlOyster::PearlOyster()
        : BonusItem(BonusType::PearlOyster, 0)
        , m_isOpen(false)
        , m_hasBlackPearl(false)
        , m_stateTimer(sf::Time::Zero)
        , m_openDuration(sf::seconds(2.0f))
        , m_closedDuration(sf::seconds(3.0f))
    {
        m_radius = 30.0f;
        m_lifetime = sf::seconds(30.0f); // Longer lifetime for oysters

        // Randomly determine pearl color - always spawn a pearl
        float r = s_pearlChance(s_randomEngine);
        if (r < 0.8f)
        {
            // White pearl
            m_hasBlackPearl = false;
            m_points = m_whitePearlPoints;
        }
        else
        {
            // Black pearl
            m_hasBlackPearl = true;
            m_points = m_blackPearlPoints;
        }
    }

    void PearlOyster::update(sf::Time deltaTime)
    {
        if (!updateLifetime(deltaTime))
            return;

        // Update open/close state
        updateOpenState(deltaTime);

        if (getSpriteComponent())
            getSpriteComponent()->update(deltaTime);

        // Bobbing animation
        m_position.y = m_baseY + computeBobbingOffset(0.5f, 0.5f);

        m_pearlSprite.setPosition(m_position);
        if (m_isOpen)
        {
            // swap to the open oyster texture and update the pearl color
            getSpriteComponent()->setTexture(*m_openTexture);
            m_pearlSprite.setTexture(m_hasBlackPearl ? *m_blackPearlTexture : *m_whitePearlTexture);
        }
        else
        {
            // revert to the closed oyster texture
            getSpriteComponent()->setTexture(*m_closedTexture);
        }
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
        if (getSpriteComponent())
            target.draw(*getSpriteComponent(), states);

        if (m_isOpen)
            target.draw(m_pearlSprite, states);
    }

    void PearlOyster::updateOpenState(sf::Time deltaTime)
    {
        m_stateTimer += deltaTime;

        if (m_isOpen)
        {
            if (m_stateTimer >= m_openDuration)
            {
                m_isOpen = false;
                m_stateTimer = sf::Time::Zero;
            }
        }
        else
        {
            if (m_stateTimer >= m_closedDuration)
            {
                m_isOpen = true;
                m_stateTimer = sf::Time::Zero;
            }
        }
    }
}