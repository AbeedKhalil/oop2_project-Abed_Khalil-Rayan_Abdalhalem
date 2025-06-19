#pragma once

#include "DrawHelpers.h"
#include <SFML/Graphics.hpp>

namespace FishGame
{
    // Mixin that provides a default draw implementation for
    // entities with a sprite component
    template<class Derived>
    class SpriteDrawable
    {
    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const
        {
            DrawUtils::drawSpriteIfPresent(static_cast<const Derived&>(*this), target, states);
        }
    };
}
