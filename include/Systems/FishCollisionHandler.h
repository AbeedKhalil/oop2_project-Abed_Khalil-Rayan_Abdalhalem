#pragma once

#include "Fish.h"
#include "Pufferfish.h"
#include "PoisonFish.h"
#include "Hazard.h"
#include "SoundPlayer.h"
#include "CollisionDetector.h"
#include <type_traits>
#include <functional>
#include <algorithm>

namespace FishGame
{
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
        static void processFishHazardCollisions(EntityContainer& entities,
            HazardContainer& hazards, SoundPlayer* soundPlayer = nullptr)
        {
            std::for_each(entities.begin(), entities.end(),
                [&hazards, soundPlayer](auto& entity)
                {
                    if (auto* fish = dynamic_cast<Fish*>(entity.get()))
                    {
                        if (!fish->isAlive()) return;

                        std::for_each(hazards.begin(), hazards.end(),
                            [fish, soundPlayer](auto& hazard)
                            {
                                if (!hazard->isAlive()) return;

                                if (CollisionDetector::checkCircleCollision(*fish, *hazard))
                                {
                                    handleFishToHazardCollision(*fish, *hazard, soundPlayer);
                                }
                            });
                    }
                });
        }

    private:
        static void handleFishToFishCollision(Fish& fish1, Fish& fish2)
        {
            // Pufferfish pushback
            if (auto* puffer1 = dynamic_cast<Pufferfish*>(&fish1))
            {
                if (puffer1->isInflated() && puffer1->canPushEntity(fish2))
                {
                    puffer1->pushEntity(fish2);
                    return;
                }
            }
            if (auto* puffer2 = dynamic_cast<Pufferfish*>(&fish2))
            {
                if (puffer2->isInflated() && puffer2->canPushEntity(fish1))
                {
                    puffer2->pushEntity(fish1);
                    return;
                }
            }

            // Handle eating
            if (fish1.canEat(fish2))
            {
                // Check if eating a poison fish
                if (auto* poisonFish = dynamic_cast<PoisonFish*>(&fish2))
                {
                    fish1.setPoisoned(poisonFish->getPoisonDuration());
                }
                fish1.playEatAnimation();
                fish2.destroy();
            }
            else if (fish2.canEat(fish1))
            {
                // Check if eating a poison fish
                if (auto* poisonFish = dynamic_cast<PoisonFish*>(&fish1))
                {
                    fish2.setPoisoned(poisonFish->getPoisonDuration());
                }
                fish2.playEatAnimation();
                fish1.destroy();
            }
        }

        static void handleFishToHazardCollision(Fish& fish, Hazard& hazard, SoundPlayer* soundPlayer)
        {
            switch (hazard.getHazardType())
            {
            case HazardType::Bomb:
                if (auto* bomb = dynamic_cast<Bomb*>(&hazard))
                {
                    bool wasExploding = bomb->isExploding();
                    bomb->onContact(fish);
                    if (!wasExploding && bomb->isExploding() && soundPlayer)
                    {
                        soundPlayer->play(SoundEffectID::MineExplode);
                    }
                }
                break;

            case HazardType::Jellyfish:
                if (auto* jellyfish = dynamic_cast<Jellyfish*>(&hazard))
                {
                    jellyfish->onContact(fish);
                    fish.setStunned(jellyfish->getStunDuration());
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
