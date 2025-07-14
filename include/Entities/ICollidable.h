#pragma once

namespace FishGame {
    class Player;
    class CollisionSystem;

    class ICollidable {
    public:
        virtual ~ICollidable() = default;
        virtual void onCollide(Player& player, CollisionSystem& system) = 0;
    };
}
