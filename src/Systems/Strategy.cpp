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

        float speed = 100.f;
        sf::Vector2f pos = entity.getPosition();
        sf::Vector2f velocity = entity.getVelocity();

        if (auto* fish = dynamic_cast<Fish*>(&entity))
        {
            speed = fish->getSpeed();

            const sf::Vector2u win = fish->getWindowBounds();
            const float margin = 60.f;

            if ((pos.x < margin && velocity.x < 0.f) ||
                (pos.x > win.x - margin && velocity.x > 0.f))
            {
                velocity.x = -velocity.x;
            }

            if ((pos.y < margin && velocity.y < 0.f) ||
                (pos.y > win.y - margin && velocity.y > 0.f))
            {
                velocity.y = -velocity.y;
            }
        }

        if (m_changeTimer <= sf::Time::Zero)
        {
            std::uniform_real_distribution<float> angleDist(0.f, 2.f * Constants::PI);
            float angle = angleDist(m_engine);

            velocity.x = std::cos(angle) * speed;
            velocity.y = std::sin(angle) * speed;

            m_changeTimer = sf::seconds(1.f);
        }

        entity.setVelocity(velocity);
        entity.updatePosition(deltaTime);
    }
}

