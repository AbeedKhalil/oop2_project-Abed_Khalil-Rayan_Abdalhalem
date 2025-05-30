#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <algorithm>
#include <memory>
#include <numeric>

namespace FishGame
{
    // Template-based particle system following OOP guidelines
    template<typename ParticleType>
    class ParticleSystem : public sf::Drawable
    {
    public:
        ParticleSystem(size_t maxParticles = 1000)
            : m_particles()
            , m_maxParticles(maxParticles)
            , m_randomEngine(std::random_device{}())
        {
            m_particles.reserve(m_maxParticles);
        }

        // Template method for creating particles with custom generator
        template<typename Generator>
        void emit(size_t count, sf::Vector2f position, Generator&& generator)
        {
            size_t availableSpace = m_maxParticles - m_particles.size();
            size_t toEmit = std::min(count, availableSpace);

            std::generate_n(std::back_inserter(m_particles), toEmit,
                [this, &position, &generator]() {
                    return generator(position, m_randomEngine);
                });
        }

        void update(sf::Time deltaTime)
        {
            // Update all particles using STL algorithm
            std::for_each(m_particles.begin(), m_particles.end(),
                [deltaTime](ParticleType& particle) {
                    particle.update(deltaTime);
                });

            // Remove dead particles using erase-remove idiom
            m_particles.erase(
                std::remove_if(m_particles.begin(), m_particles.end(),
                    [](const ParticleType& particle) {
                        return !particle.isAlive();
                    }),
                m_particles.end()
            );
        }

        size_t getActiveCount() const { return m_particles.size(); }
        void clear() { m_particles.clear(); }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override
        {
            std::for_each(m_particles.begin(), m_particles.end(),
                [&target, &states](const ParticleType& particle) {
                    particle.draw(target, states);
                });
        }

    private:
        std::vector<ParticleType> m_particles;
        size_t m_maxParticles;
        std::mt19937 m_randomEngine;
    };

    // Basic particle structure
    struct BasicParticle
    {
        sf::CircleShape shape;
        sf::Vector2f velocity;
        sf::Time lifetime;
        sf::Time maxLifetime;
        float alpha;

        BasicParticle(float radius = 3.0f)
            : shape(radius)
            , velocity(0.0f, 0.0f)
            , lifetime(sf::Time::Zero)
            , maxLifetime(sf::seconds(1.0f))
            , alpha(255.0f)
        {
        }

        void update(sf::Time deltaTime)
        {
            lifetime += deltaTime;
            shape.move(velocity * deltaTime.asSeconds());

            // Update alpha based on lifetime
            float lifeRatio = lifetime / maxLifetime;
            alpha = std::max(0.0f, 255.0f * (1.0f - lifeRatio));

            sf::Color color = shape.getFillColor();
            color.a = static_cast<sf::Uint8>(alpha);
            shape.setFillColor(color);
        }

        bool isAlive() const { return lifetime < maxLifetime; }

        void draw(sf::RenderTarget& target, sf::RenderStates states) const
        {
            target.draw(shape, states);
        }
    };

    // Particle generator template
    template<typename RNG>
    struct DefaultParticleGenerator
    {
        sf::Color color;
        float minSpeed;
        float maxSpeed;
        float particleRadius;

        DefaultParticleGenerator(sf::Color c = sf::Color::White,
            float minS = 50.0f,
            float maxS = 150.0f,
            float radius = 3.0f)
            : color(c), minSpeed(minS), maxSpeed(maxS), particleRadius(radius)
        {
        }

        BasicParticle operator()(sf::Vector2f position, RNG& rng)
        {
            std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
            std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);

            BasicParticle particle(particleRadius);
            particle.shape.setPosition(position);
            particle.shape.setFillColor(color);

            float angle = angleDist(rng) * 3.14159f / 180.0f;
            float speed = speedDist(rng);
            particle.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

            return particle;
        }
    };

    // Type alias for convenience
    using BasicParticleSystem = ParticleSystem<BasicParticle>;
}