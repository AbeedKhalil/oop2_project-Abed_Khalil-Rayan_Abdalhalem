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
}
