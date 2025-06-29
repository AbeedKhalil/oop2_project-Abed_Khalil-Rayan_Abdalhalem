#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include "GameConstants.h"

namespace FishGame
{
    struct Particle
    {
        sf::CircleShape shape;
        sf::Vector2f velocity;
        sf::Time lifetime;
        float alpha = 0.f;
    };

    class ParticleSystem : public sf::Drawable
    {
    public:
        ParticleSystem();
        void update(sf::Time dt);
        void createEffect(const sf::Vector2f& pos, const sf::Color& color, int count = Constants::DEFAULT_PARTICLE_COUNT);
        void clear() { m_particles.clear(); }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        std::vector<Particle> m_particles;
        std::mt19937 m_rng;
        std::uniform_real_distribution<float> m_angleDist;
        std::uniform_real_distribution<float> m_speedDist;
    };
}
