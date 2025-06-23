#pragma once

#include "Entity.h"
#include <SFML/System/Time.hpp>

namespace FishGame
{
    class MovementStrategy
    {
    public:
        virtual ~MovementStrategy() = default;
        virtual void update(Entity& entity, sf::Time deltaTime) = 0;
    };
}

