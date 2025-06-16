#include "OysterManager.h"
#include "SpriteManager.h"
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
            }
        }

        updateAnimation(deltaTime);
        updateSprite();
    }

    void PermanentOyster::onCollect()
    {
        if (m_state == State::Open && !m_recentlyCollected)
        {
            m_recentlyCollected = true;
            m_collectionCooldown = sf::seconds(m_cooldownDuration);

            // Hide pearl
            m_hasPearlSprite = false;
            m_points = 0;

            // Begin closing
            m_state = State::Closing;
            m_frameTimer = sf::Time::Zero;
            m_stateTimer = sf::Time::Zero;
        }
    }

    void PermanentOyster::reset()
    {
        m_recentlyCollected = false;
        m_collectionCooldown = sf::Time::Zero;
        m_state = State::Closed;
        m_frame = 0;
        m_stateTimer = sf::Time::Zero;
        m_frameTimer = sf::Time::Zero;
        m_hasPearlSprite = false;
    }

    void PermanentOyster::initializeSprites(SpriteManager& spriteManager)
    {
        m_oysterTexture = &spriteManager.getTexture(TextureID::PearlOysterClosed);
        m_whitePearlTex = &spriteManager.getTexture(TextureID::WhitePearl);
        m_blackPearlTex = &spriteManager.getTexture(TextureID::BlackPearl);

        m_sprite.setTexture(*m_oysterTexture);
        m_sprite.setOrigin(50.f, 50.f);
        m_pearlSprite.setOrigin(16.f, 16.f);
        m_pearlSprite.setScale(0.85f, 0.85f); // smaller pearl sprite
        updateSprite();
    }

    void PermanentOyster::updateAnimation(sf::Time dt)
    {
        const sf::Time frameDur = sf::seconds(m_frameTime);

        switch (m_state)
        {
        case State::Closed:
            m_stateTimer += dt;
            if (m_stateTimer >= m_closedDuration)
            {
                m_state = State::Opening;
                m_stateTimer = sf::Time::Zero;
                m_frame = 0;
            }
            break;
        case State::Opening:
            m_frameTimer += dt;
            if (m_frameTimer >= frameDur)
            {
                m_frameTimer -= frameDur;
                if (++m_frame >= m_frameCount - 1)
                {
                    m_frame = m_frameCount - 1;
                    m_state = State::Open;
                    m_stateTimer = sf::Time::Zero;
                    spawnPearl();
                }
            }
            break;
        case State::Open:
            m_stateTimer += dt;
            if (m_stateTimer >= m_openDuration)
            {
                m_state = State::Closing;
                m_stateTimer = sf::Time::Zero;
                m_frameTimer = sf::Time::Zero;
            }
            break;
        case State::Closing:
            m_frameTimer += dt;
            if (m_frameTimer >= frameDur)
            {
                m_frameTimer -= frameDur;
                if (--m_frame <= 0)
                {
                    m_frame = 0;
                    m_state = State::Closed;
                    m_stateTimer = sf::Time::Zero;
                    m_hasPearlSprite = false;
                }
            }
            break;
        }
    }

    void PermanentOyster::updateSprite()
    {
        if (!m_oysterTexture)
            return;

        const int frameW = 101;
        const int frameH = 101;
        sf::IntRect rect(1 + m_frame * frameW, 1, frameW, frameH);
        m_sprite.setTextureRect(rect);
        m_sprite.setPosition(m_position);

        if (m_hasPearlSprite)
        {
            m_pearlSprite.setPosition(m_position);
        }
    }

    void PermanentOyster::spawnPearl()
    {
        float r = s_pearlChance(s_randomEngine);
        if (r < 0.60f)
        {
            m_hasBlackPearl = false;
            m_points = m_whitePearlPoints;  
            m_pearlSprite.setTexture(*m_whitePearlTex);
            m_hasPearlSprite = true;
        }
        else if (r < 0.65f)
        {
            m_hasBlackPearl = true;
            m_points = m_blackPearlPoints;
            m_pearlSprite.setTexture(*m_blackPearlTex);
            m_hasPearlSprite = true;
        }
        else
        {
            m_hasPearlSprite = false;
            m_points = 0;
            m_hasBlackPearl = false;
        }
    }

    void PermanentOyster::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_sprite, states);
        if (m_hasPearlSprite)
            target.draw(m_pearlSprite, states);
    }
}