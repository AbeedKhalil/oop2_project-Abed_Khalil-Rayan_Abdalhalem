#include "Strategy.h"
#include "Fish.h"
#include <cmath>

namespace FishGame
{
    AggressiveChaseStrategy::AggressiveChaseStrategy(Entity* target)
        : m_target(target)
    {
    }

    void AggressiveChaseStrategy::update(Entity& entity, sf::Time deltaTime)
    {
        if (!m_target)
        {
            entity.updatePosition(deltaTime);
            return;
        }

        sf::Vector2f direction = m_target->getPosition() - entity.getPosition();
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0.f)
        {
            direction /= length;
        }
        float speed = 100.f;
        if (auto* fish = dynamic_cast<Fish*>(&entity))
            speed = fish->getSpeed();
        entity.setVelocity(direction * speed);
        entity.updatePosition(deltaTime);
    }


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

