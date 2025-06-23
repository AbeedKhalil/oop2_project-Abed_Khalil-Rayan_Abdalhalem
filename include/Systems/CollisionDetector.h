#pragma once

#include "Entity.h"
#include <cmath>

namespace FishGame
{
    // Utility class for collision detection
    class CollisionDetector
    {
    public:
        // Delete constructor - this is a utility class with only static methods
        CollisionDetector() = delete;

        // Check collision between two circular entities
        template<EntityUtils::CircularEntity A, EntityUtils::CircularEntity B>
        static bool checkCircleCollision(const A& entity1, const B& entity2)
        {
            return EntityUtils::areColliding(entity1, entity2);
        }


        // Get distance between two points
        static float getDistance(const sf::Vector2f& point1, const sf::Vector2f& point2)
        {
            float dx = point1.x - point2.x;
            float dy = point1.y - point2.y;
            return std::sqrt(dx * dx + dy * dy);
        }

        // Get distance between two entities
        template<EntityUtils::PositionProvider A, EntityUtils::PositionProvider B>
        static float getDistance(const A& entity1, const B& entity2)
        {
            return EntityUtils::distance(entity1, entity2);
        }

    };
}