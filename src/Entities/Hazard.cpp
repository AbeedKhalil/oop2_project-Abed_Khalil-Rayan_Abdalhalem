#include "Hazard.h"
#include "Player.h"
#include "GameConstants.h"
#include "SpriteManager.h"
#include "Utils/AnimatedSprite.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iterator>

namespace FishGame
{
    // Hazard base class implementation
    Hazard::Hazard(HazardType type, float damageAmount)
        : Entity()
        , m_hazardType(type)
        , m_damageAmount(damageAmount)
    {
    }

    // Bomb implementation
    Bomb::Bomb()
        : Hazard(HazardType::Bomb, 1.0f)
        , m_sprite(nullptr)
        , m_state(State::IdleBomb)
        , m_puffLoops(0)
        , m_isExploding(false)
        , m_stateTimer(sf::Time::Zero)
        , m_explosionRadius(0.f)
    {
        m_radius = m_baseRadius;
    }

    void Bomb::initializeSprite(SpriteManager& spriteManager)
    {
        const sf::Texture& tex = spriteManager.getTexture(TextureID::Bomb);
        m_sprite = std::make_unique<AnimatedSprite>(tex);

        AnimatedSprite::Animation idle;
        idle.frames.push_back(sf::IntRect(1, 1, 69, 69));
        idle.frameTime = sf::seconds(0.1f);

        AnimatedSprite::Animation explode;
        explode.frames = {
            {1, 70, 164, 146},
            {165, 70, 164, 146},
            {329, 70, 164, 146},
            {493, 70, 164, 146},
            {657, 70, 164, 146}
        };
        explode.frameTime = sf::seconds(0.08f);
        explode.loop = false;

        AnimatedSprite::Animation puffs;
        puffs.frames = {
            {1, 216, 86, 84},
            {87, 216, 86, 84}
        };
        puffs.frameTime = sf::seconds(m_puffFrameTime);
        puffs.loop = false;

        AnimatedSprite::Animation smoke;
        smoke.frames.push_back(sf::IntRect(1, 300, 122, 121));
        smoke.frameTime = sf::seconds(0.2f);
        smoke.loop = false;

        m_sprite->addAnimation("idle", idle);
        m_sprite->addAnimation("explode", explode);
        m_sprite->addAnimation("puffs", puffs);
        m_sprite->addAnimation("smoke", smoke);
        m_sprite->play("idle");
        m_sprite->setPosition(m_position);
    }

    void Bomb::update(sf::Time deltaTime)
    {
        if (!m_isAlive || !m_sprite)
            return;

        m_sprite->setPosition(m_position);
        m_sprite->update(deltaTime);

        m_stateTimer += deltaTime;

        switch (m_state)
        {
        case State::IdleBomb:
            break;
        case State::Explode:
        {
            float progress = m_stateTimer.asSeconds() / m_explosionDuration;
            progress = std::clamp(progress, 0.f, 1.f);
            m_explosionRadius = m_baseRadius +
                (m_maxExplosionRadius - m_baseRadius) * progress;
            m_isExploding = progress < 1.f;
            if (m_sprite->isFinished())
                advanceState();
            break;
        }
        case State::Puffs:
            if (m_sprite->isFinished())
            {
                ++m_puffLoops;
                if (m_puffLoops >= m_maxPuffLoops)
                    advanceState();
                else
                    m_sprite->play("puffs");
            }
            break;
        case State::Smoke:
            if (m_sprite->isFinished())
                advanceState();
            break;
        default:
            break;
        }
    }

    sf::FloatRect Bomb::getBounds() const
    {
        float effectiveRadius = m_isExploding ? m_explosionRadius : m_radius;
        return sf::FloatRect(m_position.x - effectiveRadius, m_position.y - effectiveRadius,
            effectiveRadius * 2.f, effectiveRadius * 2.f);
    }

    void Bomb::onContact(Entity& entity)
    {
        (void)entity;
        trigger();
    }

    void Bomb::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (m_sprite)
            target.draw(*m_sprite, states);
    }

    void Bomb::advanceState()
    {
        m_stateTimer = sf::Time::Zero;

        switch (m_state)
        {
        case State::IdleBomb:
            m_state = State::Explode;
            m_sprite->play("explode");
            m_isExploding = true;
            break;
        case State::Explode:
            m_state = State::Puffs;
            m_sprite->play("puffs");
            m_isExploding = false;
            m_puffLoops = 0;
            break;
        case State::Puffs:
            m_state = State::Smoke;
            m_sprite->play("smoke");
            break;
        case State::Smoke:
            m_state = State::Done;
            destroy();
            break;
        default:
            break;
        }
    }

    void Bomb::trigger()
    {
        if (m_state == State::IdleBomb)
            advanceState();
    }

    bool Bomb::isFinished() const
    {
        return m_state == State::Done;
    }

    // Jellyfish implementation
    Jellyfish::Jellyfish()
        : Hazard(HazardType::Jellyfish, 0.5f)
        , m_bell(15.0f)
        , m_tentacles()
        , m_floatAnimation(0.0f)
        , m_tentacleWave(0.0f)
        , m_stunDuration(sf::seconds(m_stunEffectDuration))
        , m_frame(0)
        , m_frameTimer(sf::Time::Zero)
        , m_frameWidth(0)
    {
        m_radius = 15.0f;

        // Translucent bell
        m_bell.setFillColor(sf::Color(255, 200, 255, 150));
        m_bell.setOutlineColor(sf::Color(255, 150, 255));
        m_bell.setOutlineThickness(1.0f);
        m_bell.setOrigin(m_radius, m_radius);

        // Create tentacles
        m_tentacles.reserve(m_tentacleCount);
        std::generate_n(std::back_inserter(m_tentacles), m_tentacleCount, [] {
            sf::RectangleShape tentacle(sf::Vector2f(2.0f, 30.0f));
            tentacle.setFillColor(sf::Color(255, 150, 255, 100));
            tentacle.setOrigin(1.0f, 0.0f);
            return tentacle;
            });
    }

    void Jellyfish::initializeSprite(SpriteManager& spriteManager)
    {
        auto sprite = spriteManager.createSpriteComponent(
            static_cast<Entity*>(this), TextureID::Jellyfish);
        if (sprite)
        {
            auto config = spriteManager.getSpriteConfig<Entity>(TextureID::Jellyfish);
            sprite->configure(config);
            setSpriteComponent(std::move(sprite));
            setRenderMode(RenderMode::Sprite);

            // Set initial frame
            m_texture = &spriteManager.getTexture(TextureID::Jellyfish);
            m_frameWidth = static_cast<int>(m_texture->getSize().x) - 2;
            sf::IntRect rect(1, 1, m_frameWidth, m_frameHeight);
            getSpriteComponent()->setTextureRect(rect);
        }
    }

    void Jellyfish::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        if (getRenderMode() == RenderMode::Sprite && getSpriteComponent())
        {
            getSpriteComponent()->update(deltaTime);

            // Advance sprite animation
            m_frameTimer += deltaTime;
            if (m_frameTimer.asSeconds() >= m_frameTime)
            {
                m_frameTimer -= sf::seconds(m_frameTime);
                m_frame = (m_frame + 1) % m_frameCount;
                if (m_texture)
                {
                    int y = 1 + m_frame * m_frameHeight;
                    sf::IntRect rect(1, y, m_frameWidth, m_frameHeight);
                    getSpriteComponent()->setTextureRect(rect);
                }
            }
        }

        // Floating movement
        m_floatAnimation += deltaTime.asSeconds() * 2.0f;
        m_tentacleWave += deltaTime.asSeconds() * 3.0f;

        float floatOffset = std::sin(m_floatAnimation) * 10.0f;
        m_position.y += floatOffset * deltaTime.asSeconds();

        // Drift movement
        m_position += m_velocity * deltaTime.asSeconds();

        // Update bell
        m_bell.setPosition(m_position);

        // Update tentacles with wave motion
        for (size_t i = 0; i < m_tentacles.size(); ++i)
        {
            float angle = (360.0f / m_tentacleCount) * i * Constants::DEG_TO_RAD;
            float wave = std::sin(m_tentacleWave + i * 0.5f) * 10.0f;

            sf::Vector2f tentaclePos(
                m_position.x + std::cos(angle) * 15.0f,
                m_position.y + std::sin(angle) * 15.0f
            );

            m_tentacles[i].setPosition(tentaclePos);
            m_tentacles[i].setRotation((angle * Constants::RAD_TO_DEG) + 90.0f + wave);
        }

        // Check boundaries
        if (m_position.y > static_cast<float>(Constants::WINDOW_HEIGHT) + 100.0f)
        {
            m_position.y = -100.0f;
        }
    }

    sf::FloatRect Jellyfish::getBounds() const
    {
        // Include tentacle reach
        float effectiveRadius = m_radius + 15.0f;
        return sf::FloatRect(m_position.x - effectiveRadius, m_position.y - effectiveRadius,
            effectiveRadius * 2.0f, effectiveRadius * 2.0f);
    }

    void Jellyfish::onContact(Entity& entity)
    {
        // Push any colliding entity away from the jellyfish
        pushEntity(entity);
        // Actual stun application is handled externally
    }

    void Jellyfish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (getRenderMode() == RenderMode::Sprite && getSpriteComponent())
    {
            target.draw(*getSpriteComponent(), states);
        }
        else
        {
            std::for_each(m_tentacles.begin(), m_tentacles.end(),
                [&target, &states](const sf::RectangleShape& tentacle) {
                    target.draw(tentacle, states);
                });

            target.draw(m_bell, states);
        }
    }

    void Jellyfish::pushEntity(Entity& entity) const
    {
        sf::Vector2f dir = entity.getPosition() - m_position;
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length == 0.0f)
        {
            dir = { 0.0f, 1.0f };
            length = 1.0f;
        }

        dir /= length;

        entity.setVelocity(dir * m_pushForce);
        entity.setPosition(entity.getPosition() + dir * m_pushDistance);
    }
}
