#pragma once

#include "State.h"
#include "GameConstants.h"
#include <SFML/Graphics.hpp>

namespace FishGame
{
    class PauseState;
    template<> struct is_state<PauseState> : std::true_type {};

    class PauseState : public State
    {
    public:
        explicit PauseState(Game& game);
        ~PauseState() override = default;

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;
        void onActivate() override;

    private:
        sf::Text m_pauseText;
        sf::Text m_instructionText;
        sf::RectangleShape m_background;
    };
}