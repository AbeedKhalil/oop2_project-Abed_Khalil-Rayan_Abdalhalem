#pragma once

#include "Fish.h"
#include "SpecialFish.h"
#include "Hazard.h"
#include "CollisionDetector.h"
#include <type_traits>
#include <functional>
#include <algorithm>

namespace FishGame
{
    template<typename T>
    struct is_hazard : std::false_type {};

    template<>
    struct is_hazard<Hazard> : std::true_type {};

    template<typename T>
    inline constexpr bool is_hazard_v = is_hazard<T>::value;

    class FishCollisionHandler
    {
    public:
        // Template method for handling fish-to-fish collisions
        template<typename Container>
        static void processFishCollisions(Container& entities)
        {
            std::for_each(entities.begin(), entities.end(),
                [&entities](auto& entity1)
                {
                    if (auto* fish1 = dynamic_cast<Fish*>(entity1.get()))
                    {
                        if (!fish1->isAlive() || fish1->isStunned()) return;

                        std::for_each(entities.begin(), entities.end(),
                            [fish1, &entity1](auto& entity2)
                            {
                                if (entity1 == entity2) return;

                                if (auto* fish2 = dynamic_cast<Fish*>(entity2.get()))
                                {
                                    if (!fish2->isAlive()) return;

                                    if (CollisionDetector::checkCircleCollision(*fish1, *fish2))
                                    {
                                        handleFishToFishCollision(*fish1, *fish2);
                                    }
                                }
                            });
                    }
                });
        }

        // Template method for fish-to-hazard collisions
        template<typename EntityContainer, typename HazardContainer>
        static void processFishHazardCollisions(EntityContainer& entities, HazardContainer& hazards)
        {
            std::for_each(entities.begin(), entities.end(),
                [&hazards](auto& entity)
                {
                    if (auto* fish = dynamic_cast<Fish*>(entity.get()))
                    {
                        if (!fish->isAlive()) return;

                        std::for_each(hazards.begin(), hazards.end(),
                            [fish](auto& hazard)
                            {
                                if (!hazard->isAlive()) return;

                                if (CollisionDetector::checkCircleCollision(*fish, *hazard))
                                {
                                    handleFishToHazardCollision(*fish, *hazard);
                                }
                            });
                    }
                });
        }

    private:
        static void handleFishToFishCollision(Fish& fish1, Fish& fish2)
        {
            // Handle eating
            if (fish1.canEat(fish2))
            {
                // Check if eating a poison fish
                if (auto* poisonFish = dynamic_cast<PoisonFish*>(&fish2))
                {
                    fish1.setPoisoned(poisonFish->getPoisonDuration());
                }
                fish2.destroy();
            }
            else if (fish2.canEat(fish1))
            {
                // Check if eating a poison fish
                if (auto* poisonFish = dynamic_cast<PoisonFish*>(&fish1))
                {
                    fish2.setPoisoned(poisonFish->getPoisonDuration());
                }
                fish1.destroy();
            }
        }

        static void handleFishToHazardCollision(Fish& fish, Hazard& hazard)
        {
            switch (hazard.getHazardType())
            {
            case HazardType::Bomb:
                if (auto* bomb = dynamic_cast<Bomb*>(&hazard))
                {
                    bomb->onContact(fish);
                }
                break;

            case HazardType::Jellyfish:
                if (auto* jellyfish = dynamic_cast<Jellyfish*>(&hazard))
                {
                    fish.setStunned(jellyfish->getStunDuration());
                    jellyfish->onContact(fish);
                }
                break;
            }
        }
    };

    // Template function for handling bomb explosions
    template<typename Container>
    void processBombExplosions(Container& entities, const std::vector<std::unique_ptr<Hazard>>& hazards)
    {
        std::for_each(hazards.begin(), hazards.end(),
            [&entities](const auto& hazard)
            {
                if (auto* bomb = dynamic_cast<Bomb*>(hazard.get()))
                {
                    if (bomb->isExploding())
                    {
                        sf::Vector2f bombPos = bomb->getPosition();
                        float explosionRadius = bomb->getExplosionRadius();

                        std::for_each(entities.begin(), entities.end(),
                            [bombPos, explosionRadius](auto& entity)
                            {
                                if (!entity->isAlive()) return;

                                float distance = CollisionDetector::getDistance(
                                    bombPos, entity->getPosition());

                                if (distance < explosionRadius)
                                {
                                    entity->destroy();
                                }
                            });
                    }
                }
            });
    }
}