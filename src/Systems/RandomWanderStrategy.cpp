#include "RandomWanderStrategy.h"
#include "Fish.h"
#include <cmath>

namespace FishGame
{
    RandomWanderStrategy::RandomWanderStrategy()
        : m_changeTimer(sf::seconds(0.f))
        , m_engine(std::random_device{}())
    {
    }

    void RandomWanderStrategy::update(Entity& entity, sf::Time deltaTime)
    {
        m_changeTimer -= deltaTime;
        if (m_changeTimer <= sf::Time::Zero)
        {
            std::uniform_real_distribution<float> angleDist(0.f, 2.f * 3.14159265f);
            float angle = angleDist(m_engine);
            float speed = 100.f;
            if (auto* fish = dynamic_cast<Fish*>(&entity))
                speed = fish->getSpeed();
            entity.setVelocity(std::cos(angle) * speed, std::sin(angle) * speed);
            m_changeTimer = sf::seconds(1.f);
        }
        entity.updatePosition(deltaTime);
    }
}

