#pragma once

#include <SFML/Graphics.hpp>

namespace FishGame
{
    class Player;

    class PlayerVisual
    {
    public:
        explicit PlayerVisual(Player& player);
        void update(sf::Time deltaTime);
        void triggerEatEffect();
        void triggerDamageEffect();
        void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    private:
        Player& m_player;
    };
}
