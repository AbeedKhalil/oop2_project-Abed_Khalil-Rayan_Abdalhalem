#pragma once

#include <SFML/Graphics.hpp>

namespace FishGame {
    class PlayState;

    // Separate class responsible for PlayState game logic.
    class PlayLogic
    {
    public:
        explicit PlayLogic(PlayState& state);

        void handleEvent(const sf::Event& event);
        bool update(sf::Time deltaTime);

    private:
        PlayState& m_state;
    };
}

