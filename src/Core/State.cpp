#include "State.h"
#include "Game.h"

namespace FishGame
{
    State::State(Game& game)
        : m_game(game)
    {
    }

    void State::requestStackPush(StateID stateID)
    {
        m_game.pushState(stateID);
    }

    void State::requestStackPop()
    {
        m_game.popState();
    }

    void State::requestStackClear()
    {
        m_game.clearStates();
    }
}