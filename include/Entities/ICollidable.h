#pragma once

namespace FishGame {
    class Player;
    class Fish;
    class Hazard;
    class BonusItem;
    class Entity;
    class CollisionSystem;

    class ICollidable {
    public:
        virtual ~ICollidable() = default;

        virtual void onCollide(Player& player, CollisionSystem& system) = 0;

        // Generic double dispatch entry point
        virtual void onCollideWith(Entity& entity, CollisionSystem& system) {}

        // Overloads for specific entity types
        virtual void onCollideWith(Player& player, CollisionSystem& system)
        {
            onCollide(player, system);
        }
        virtual void onCollideWith(Fish&, CollisionSystem&) {}
        virtual void onCollideWith(Hazard&, CollisionSystem&) {}
        virtual void onCollideWith(BonusItem&, CollisionSystem&) {}
    };
}
