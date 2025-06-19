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

    // Helper base that automatically provides the required draw override
    // for entity classes. This removes the need for each entity to
    // implement a trivial draw method that simply delegates to
    // SpriteDrawable.
    template<class Derived>
    class AutoSpriteDrawable : public SpriteDrawable<Derived>
    {
    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override
        {
            SpriteDrawable<Derived>::draw(target, states);
        }
    };
}
