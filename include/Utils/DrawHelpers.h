#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>

namespace FishGame::DrawUtils {
    template<typename Container>
    void drawContainer(const Container& container, sf::RenderTarget& target,
        sf::RenderStates states = {})
    {
        std::for_each(container.begin(), container.end(),
            [&target, &states](const auto& drawable) {
                target.draw(drawable, states);
            });
    }

    // Draw an entity's sprite component if one exists
    template<typename EntityType>
    void drawSpriteIfPresent(const EntityType& entity, sf::RenderTarget& target,
        sf::RenderStates states = {})
    {
        if (auto sprite = entity.getSpriteComponent())
            target.draw(*sprite, states);
    }
}

