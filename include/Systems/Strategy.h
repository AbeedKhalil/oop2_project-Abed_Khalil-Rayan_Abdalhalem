#pragma once

#include "Entity.h"
#include "GameConstants.h"
#include <random>
#include <SFML/System/Time.hpp>

namespace FishGame
{
    class MovementStrategy
    {
    public:
        virtual ~MovementStrategy() = default;
        virtual void update(Entity& entity, sf::Time deltaTime) = 0;
    };

    class RandomWanderStrategy : public MovementStrategy
    {
    public:
        RandomWanderStrategy();
        void update(Entity& entity, sf::Time deltaTime) override;

    private:
        sf::Time m_changeTimer;
        std::default_random_engine m_engine;
    };

    class AggressiveChaseStrategy : public MovementStrategy
    {
    public:
        AggressiveChaseStrategy(Entity* target);
        void update(Entity& entity, sf::Time deltaTime) override;

    private:
        Entity* m_target;
    };
}
