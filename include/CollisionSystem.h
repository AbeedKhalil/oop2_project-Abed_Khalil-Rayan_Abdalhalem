#pragma once

#include <vector>
#include <functional>
#include <algorithm>
#include <type_traits>
#include "Entity.h"

namespace FishGame
{
    // Collision strategies
    struct CircleCollision
    {
        template<typename T, typename U>
        static bool check(const T& a, const U& b)
        {
            sf::Vector2f diff = a.getPosition() - b.getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;
            float radiusSum = a.getRadius() + b.getRadius();
            return distSq < radiusSum * radiusSum;
        }
    };

    struct RectCollision
    {
        template<typename T, typename U>
        static bool check(const T& a, const U& b)
        {
            return a.getBounds().intersects(b.getBounds());
        }
    };

    // Generic collision system
    template<typename CollisionStrategy = CircleCollision>
    class CollisionSystem
    {
    public:
        using CollisionCallback = std::function<void(Entity&, Entity&)>;

        // Check collisions between single entity and container
        template<typename Container>
        void checkCollisions(Entity& entity, Container& container, CollisionCallback callback)
        {
            std::for_each(container.begin(), container.end(),
                [&entity, &callback](auto& other) {
                    if (other && other->isAlive() &&
                        CollisionStrategy::check(entity, *other))
                    {
                        callback(entity, *other);
                    }
                });
        }

        // Check all-pairs collisions in container
        template<typename Container>
        void checkAllPairs(Container& container, CollisionCallback callback)
        {
            for (auto it1 = container.begin(); it1 != container.end(); ++it1)
            {
                if (!*it1 || !(*it1)->isAlive())
                    continue;

                for (auto it2 = std::next(it1); it2 != container.end(); ++it2)
                {
                    if (!*it2 || !(*it2)->isAlive())
                        continue;

                    if (CollisionStrategy::check(**it1, **it2))
                    {
                        callback(**it1, **it2);
                    }
                }
            }
        }
    };
}