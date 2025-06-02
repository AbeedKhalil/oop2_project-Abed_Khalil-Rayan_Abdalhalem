#include "Hazard.h"
#include "Player.h"
#include <cmath>
#include <numeric>

namespace FishGame
{
    // Hazard base class implementation
    Hazard::Hazard(HazardType type, float damageAmount)
        : Entity()
        , m_hazardType(type)
        , m_damageAmount(damageAmount)
        , m_activationDelay(sf::Time::Zero)
        , m_isActivated(true)
    {
    }

    // Bomb implementation
    Bomb::Bomb()
        : Hazard(HazardType::Bomb, 1.0f)
        , m_bombShape(m_baseRadius)
        , m_fuseGlow(m_baseRadius * 1.5f)
        , m_explosionParticles()
        , m_isExploding(false)
        , m_explosionTimer(sf::Time::Zero)
        , m_explosionRadius(0.0f)
        , m_pulseAnimation(0.0f)
    {
        m_radius = m_baseRadius;

        // Setup bomb appearance
        m_bombShape.setFillColor(sf::Color(50, 50, 50));
        m_bombShape.setOutlineColor(sf::Color::Red);
        m_bombShape.setOutlineThickness(2.0f);
        m_bombShape.setOrigin(m_baseRadius, m_baseRadius);

        // Setup fuse glow
        m_fuseGlow.setFillColor(sf::Color(255, 100, 0, 100));
        m_fuseGlow.setOrigin(m_baseRadius * 1.5f, m_baseRadius * 1.5f);

        // Pre-allocate explosion particles
        m_explosionParticles.reserve(16);
    }

    void Bomb::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

        if (!m_isExploding)
        {
            // Fuse animation
            m_pulseAnimation += deltaTime.asSeconds() * 5.0f;
            float pulse = 1.0f + 0.3f * std::sin(m_pulseAnimation);

            m_bombShape.setPosition(m_position);
            m_fuseGlow.setPosition(m_position);
            m_fuseGlow.setScale(pulse, pulse);

            // Auto-explode after fuse duration
            m_explosionTimer += deltaTime;
            if (m_explosionTimer.asSeconds() >= m_fuseDuration)
            {
                m_isExploding = true;
                m_explosionTimer = sf::Time::Zero;
                createExplosion();
            }
        }
        else
        {
            // Explosion animation
            m_explosionTimer += deltaTime;
            float explosionProgress = m_explosionTimer.asSeconds() / 0.5f;

            if (explosionProgress >= 1.0f)
            {
                destroy();
                return;
            }

            m_explosionRadius = m_maxExplosionRadius * explosionProgress;

            // Update explosion particles
            std::for_each(m_explosionParticles.begin(), m_explosionParticles.end(),
                [this, explosionProgress](sf::CircleShape& particle) {
                    float scale = (1.0f - explosionProgress) * 2.0f;
                    particle.setScale(scale, scale);

                    sf::Color color = particle.getFillColor();
                    color.a = static_cast<sf::Uint8>(255 * (1.0f - explosionProgress));
                    particle.setFillColor(color);
                });
        }
    }

    sf::FloatRect Bomb::getBounds() const
    {
        float effectiveRadius = m_isExploding ? m_explosionRadius : m_radius;
        return sf::FloatRect(m_position.x - effectiveRadius, m_position.y - effectiveRadius,
            effectiveRadius * 2.0f, effectiveRadius * 2.0f);
    }

    void Bomb::onContact(Entity& entity)
    {
        if (!m_isExploding && entity.getType() == EntityType::Player)
        {
            m_isExploding = true;
            m_explosionTimer = sf::Time::Zero;
            createExplosion();
        }
    }

    void Bomb::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (!m_isExploding)
        {
            target.draw(m_fuseGlow, states);
            target.draw(m_bombShape, states);
        }
        else
        {
            std::for_each(m_explosionParticles.begin(), m_explosionParticles.end(),
                [&target, &states](const sf::CircleShape& particle) {
                    target.draw(particle, states);
                });
        }
    }

    void Bomb::createExplosion()
    {
        m_explosionParticles.clear();

        for (int i = 0; i < 16; ++i)
        {
            sf::CircleShape particle(5.0f);

            float angle = (360.0f / 16.0f) * i * 3.14159f / 180.0f;
            float radius = 30.0f;

            particle.setPosition(
                m_position.x + std::cos(angle) * radius,
                m_position.y + std::sin(angle) * radius
            );

            particle.setFillColor(sf::Color(255, 200, 0));
            particle.setOrigin(5.0f, 5.0f);

            m_explosionParticles.push_back(particle);
        }
    }

    // Jellyfish implementation
    Jellyfish::Jellyfish()
        : Hazard(HazardType::Jellyfish, 0.5f)
        , m_bell(25.0f)
        , m_tentacles()
        , m_floatAnimation(0.0f)
        , m_tentacleWave(0.0f)
        , m_stunDuration(sf::seconds(m_stunEffectDuration))
    {
        m_radius = 25.0f;

        // Translucent bell
        m_bell.setFillColor(sf::Color(255, 200, 255, 150));
        m_bell.setOutlineColor(sf::Color(255, 150, 255));
        m_bell.setOutlineThickness(1.0f);
        m_bell.setOrigin(m_radius, m_radius);

        // Create tentacles
        m_tentacles.reserve(m_tentacleCount);
        for (int i = 0; i < m_tentacleCount; ++i)
        {
            sf::RectangleShape tentacle(sf::Vector2f(2.0f, 40.0f));
            tentacle.setFillColor(sf::Color(255, 150, 255, 100));
            tentacle.setOrigin(1.0f, 0.0f);
            m_tentacles.push_back(tentacle);
        }
    }

    void Jellyfish::update(sf::Time deltaTime)
    {
        if (!m_isAlive)
            return;

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
            float angle = (360.0f / m_tentacleCount) * i * 3.14159f / 180.0f;
            float wave = std::sin(m_tentacleWave + i * 0.5f) * 10.0f;

            sf::Vector2f tentaclePos(
                m_position.x + std::cos(angle) * 20.0f,
                m_position.y + std::sin(angle) * 20.0f
            );

            m_tentacles[i].setPosition(tentaclePos);
            m_tentacles[i].setRotation((angle * 180.0f / 3.14159f) + 90.0f + wave);
        }

        // Check boundaries
        if (m_position.y > 1080.0f + 100.0f)
        {
            m_position.y = -100.0f;
        }
    }

    sf::FloatRect Jellyfish::getBounds() const
    {
        // Include tentacle reach
        float effectiveRadius = m_radius + 20.0f;
        return sf::FloatRect(m_position.x - effectiveRadius, m_position.y - effectiveRadius,
            effectiveRadius * 2.0f, effectiveRadius * 2.0f);
    }

    void Jellyfish::onContact(Entity& entity)
    {
        if (entity.getType() == EntityType::Player)
        {
            // Stun effect handled by PlayState
        }
    }

    void Jellyfish::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        std::for_each(m_tentacles.begin(), m_tentacles.end(),
            [&target, &states](const sf::RectangleShape& tentacle) {
                target.draw(tentacle, states);
            });

        target.draw(m_bell, states);
    }
}