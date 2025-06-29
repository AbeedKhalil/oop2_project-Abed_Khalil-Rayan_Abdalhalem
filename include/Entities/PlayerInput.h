#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>

namespace sf { class Keyboard; }

namespace FishGame
{
    class Player;

    class PlayerInput
    {
    public:
        explicit PlayerInput(Player& player);
        void handleInput();
    private:
        Player& m_player;
    };
}
