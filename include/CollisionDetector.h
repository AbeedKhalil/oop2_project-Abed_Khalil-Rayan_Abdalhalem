#pragma once

#include "Entity.h"
#include <cmath>
#include <algorithm>

namespace FishGame
{
    // Utility class for collision detection
    class CollisionDetector
    {
    public:
        // Delete constructor - this is a utility class with only static methods
        CollisionDetector() = delete;

        // Check collision between two circular entities
        static bool checkCircleCollision(const Entity& entity1, const Entity& entity2)
        {
            sf::Vector2f pos1 = entity1.getPosition();
            sf::Vector2f pos2 = entity2.getPosition();

            float dx = pos1.x - pos2.x;
            float dy = pos1.y - pos2.y;
            float distanceSquared = dx * dx + dy * dy;

            float radiusSum = entity1.getRadius() + entity2.getRadius();
            float radiusSumSquared = radiusSum * radiusSum;

            return distanceSquared < radiusSumSquared;
        }

        // Check if a point is inside a circle
        static bool pointInCircle(const sf::Vector2f& point, const sf::Vector2f& circleCenter, float radius)
        {
            float dx = point.x - circleCenter.x;
            float dy = point.y - circleCenter.y;
            float distanceSquared = dx * dx + dy * dy;

            return distanceSquared < (radius * radius);
        }

        // Check collision between two rectangles
        static bool checkRectangleCollision(const sf::FloatRect& rect1, const sf::FloatRect& rect2)
        {
            return rect1.intersects(rect2);
        }

        // Get distance between two points
        static float getDistance(const sf::Vector2f& point1, const sf::Vector2f& point2)
        {
            float dx = point1.x - point2.x;
            float dy = point1.y - point2.y;
            return std::sqrt(dx * dx + dy * dy);
        }

        // Get squared distance between two points (more efficient when exact distance not needed)
        static float getDistanceSquared(const sf::Vector2f& point1, const sf::Vector2f& point2)
        {
            float dx = point1.x - point2.x;
            float dy = point1.y - point2.y;
            return dx * dx + dy * dy;
        }
    };
}