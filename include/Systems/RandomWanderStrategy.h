#pragma once

#include "MovementStrategy.h"
#include <random>

namespace FishGame
{
    class RandomWanderStrategy : public MovementStrategy
    {
    public:
        RandomWanderStrategy();
        void update(Entity& entity, sf::Time deltaTime) override;

    private:
        sf::Time m_changeTimer;
        std::default_random_engine m_engine;
    };
}

