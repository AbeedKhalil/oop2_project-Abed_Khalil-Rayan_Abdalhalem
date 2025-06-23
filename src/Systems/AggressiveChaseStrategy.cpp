#include "AggressiveChaseStrategy.h"
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
}

