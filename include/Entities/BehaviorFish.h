#pragma once

#include "Entity.h"
#include "SpriteComponent.h"
#include <utility>

namespace FishGame {
    // Base behavior interface
    struct IFishBehavior {
        virtual ~IFishBehavior() = default;
        virtual void update(Entity& self, sf::Time dt) = 0;
    };

    // Template-based fish entity using a behavior strategy
    template<class Behavior>
    class BehaviorFish : public Entity {
    public:
        explicit BehaviorFish(Behavior behavior)
            : m_behavior(std::move(behavior)) {}

        void update(sf::Time dt) override {
            m_behavior.update(*this, dt);
        }

        sf::FloatRect getBounds() const override {
            if (auto sprite = getSpriteComponent())
                return sprite->getBounds();
            return sf::FloatRect();
        }

        EntityType getType() const override { return EntityType::SmallFish; }

    private:
        Behavior m_behavior;
    };

    // Example behaviors
    struct PassiveBehavior : IFishBehavior {
        void update(Entity& self, sf::Time dt) override {
            // simple idle swim behaviour
            sf::Vector2f pos = self.getPosition();
            pos.x += 10.f * dt.asSeconds();
            self.setPosition(pos);
        }
    };

    struct AggressiveBehavior : IFishBehavior {
        void update(Entity& self, sf::Time dt) override {
            // move faster toward the right
            sf::Vector2f pos = self.getPosition();
            pos.x += 50.f * dt.asSeconds();
            self.setPosition(pos);
        }
    };
}
