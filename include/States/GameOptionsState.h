#pragma once

#include "State.h"
#include "GameConstants.h"

namespace FishGame
{
    class GameOptionsState;
    template<> struct is_state<GameOptionsState> : std::true_type {};

    class GameOptionsState : public State
    {
    public:
        explicit GameOptionsState(Game& game);
        ~GameOptionsState() override = default;

        void handleEvent(const sf::Event& event) override;
        bool update(sf::Time deltaTime) override;
        void render() override;
        void onActivate() override;

    private:
        sf::Text m_titleText;
        sf::Text m_instructionText;
        sf::RectangleShape m_background;
    };
}