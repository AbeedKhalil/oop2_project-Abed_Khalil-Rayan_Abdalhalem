#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <optional>
#include <functional>
#include "CollisionDetector.h"

namespace FishGame::StateUtils
{
    // Update all alive entities in a container with optional filter
    template<typename Container, typename Predicate>
    void updateEntities(Container& container, sf::Time dt, Predicate pred)
    {
        for (auto& entity : container)
        {
            if (entity && entity->isAlive() && (!pred || pred(*entity)))
            {
                entity->update(dt);
            }
        }
    }

    template<typename Container>
    void updateEntities(Container& container, sf::Time dt)
    {
        updateEntities(container, dt, std::function<bool(const typename Container::value_type::element_type&)>());
    }

    // Apply a functor to all alive entities in a container
    template<typename Container, typename Func>
    void applyToEntities(Container& container, Func func)
    {
        std::for_each(container.begin(), container.end(),
            [&func](auto& entity)
            {
                if (entity && entity->isAlive())
                {
                    func(*entity);
                }
            });
    }

    // Remove all dead entities from a container
    template<typename Container>
    void removeDead(Container& container)
    {
        container.erase(
            std::remove_if(container.begin(), container.end(),
                [](const auto& entity)
                {
                    return !entity || !entity->isAlive();
                }),
            container.end());
    }

    // Render all alive entities
    template<typename Container>
    void renderContainer(const Container& container, sf::RenderWindow& window)
    {
        std::for_each(container.begin(), container.end(),
            [&window](const auto& entity)
            {
                if (entity && entity->isAlive())
                {
                    window.draw(*entity);
                }
            });
    }

    // Generic collision detection between two containers
    template<typename Container1, typename Container2, typename CollisionFunc>
    void processCollisionsBetween(Container1& c1, Container2& c2, CollisionFunc onCollision)
    {
        for (auto& item1 : c1)
        {
            if (!item1 || !item1->isAlive())
                continue;

            for (auto& item2 : c2)
            {
                if (!item2 || !item2->isAlive() || item1 == item2)
                    continue;

                if (CollisionDetector::checkCircleCollision(*item1, *item2))
                {
                    onCollision(*item1, *item2);
                }
            }
        }
    }

    // Collision between an entity and a container
    template<typename Entity, typename Container, typename CollisionFunc>
    void processEntityVsContainer(Entity& entity, Container& container, CollisionFunc onCollision)
    {
        if (!entity.isAlive())
            return;

        std::for_each(container.begin(), container.end(),
            [&entity, &onCollision](auto& item)
            {
                if (item && item->isAlive() && CollisionDetector::checkCircleCollision(entity, *item))
                {
                    onCollision(*item);
                }
            });
    }

    // Generic helper to find an item under the mouse cursor
    template<typename Container, typename BoundsFunc>
    std::optional<size_t> findItemAt(const Container& items, const sf::Vector2f& pos, BoundsFunc getBounds)
    {
        auto it = std::find_if(items.begin(), items.end(),
            [&pos, &getBounds](const auto& item)
            {
                return getBounds(item).contains(pos);
            });

        if (it != items.end())
            return static_cast<size_t>(std::distance(items.begin(), it));

        return std::nullopt;
    }

    // Simple pulse effect for anything supporting setScale
    template<typename Drawable>
    void applyPulseEffect(Drawable& drawable, float scale)
    {
        drawable.setScale(scale, scale);
    }
}