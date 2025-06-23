#pragma once

#include "MovementStrategy.h"

namespace FishGame
{
    class AggressiveChaseStrategy : public MovementStrategy
    {
    public:
        AggressiveChaseStrategy(Entity* target);
        void update(Entity& entity, sf::Time deltaTime) override;

    private:
        Entity* m_target;
    };
}

