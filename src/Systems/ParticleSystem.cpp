#include "ParticleSystem.h"
#include <algorithm>
#include <execution>
#include <cmath>

namespace FishGame
{
    ParticleSystem::ParticleSystem()
        : m_particles()
        , m_rng(std::random_device{}())
        , m_angleDist(0.f, 360.f)
        , m_speedDist(Constants::MIN_PARTICLE_SPEED, Constants::MAX_PARTICLE_SPEED)
    {
        m_particles.reserve(Constants::MAX_PARTICLES);
    }

    void ParticleSystem::update(sf::Time dt)
    {
        for (auto& p : m_particles)
        {
            p.lifetime -= dt;
            p.shape.move(p.velocity * dt.asSeconds());
            p.alpha = std::max(0.f, p.alpha - Constants::PARTICLE_FADE_RATE * dt.asSeconds());
            sf::Color c = p.shape.getFillColor();
            c.a = static_cast<sf::Uint8>(p.alpha);
            p.shape.setFillColor(c);
        }
        m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
            [](const Particle& p){ return p.lifetime <= sf::Time::Zero; }), m_particles.end());
    }

    void ParticleSystem::createEffect(const sf::Vector2f& pos, const sf::Color& color, int count)
    {
        m_particles.reserve(m_particles.size() + count);
        for(int i=0;i<count;++i)
        {
            Particle p;
            p.shape = sf::CircleShape(Constants::PARTICLE_RADIUS);
            p.shape.setFillColor(color);
            p.shape.setPosition(pos);
            float angle = m_angleDist(m_rng) * Constants::DEG_TO_RAD;
            float speed = m_speedDist(m_rng);
            p.velocity = {std::cos(angle)*speed, std::sin(angle)*speed};
            p.lifetime = sf::seconds(Constants::PARTICLE_LIFETIME);
            p.alpha = Constants::PARTICLE_INITIAL_ALPHA;
            m_particles.push_back(p);
        }
    }

    void ParticleSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        for(const auto& p : m_particles)
            target.draw(p.shape, states);
    }
}
