#include "State.h"
#include "Game.h"
#include <algorithm>

namespace FishGame
{
    State::State(Game& game)
        : m_game(game)
        , m_deferredActions()
    {
        m_deferredActions.reserve(10); // Pre-allocate for typical usage
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

    void State::processDeferredActions()
    {
        // Execute all deferred actions
        std::for_each(m_deferredActions.begin(), m_deferredActions.end(),
            [](const auto& action) { action(); });

        m_deferredActions.clear();
    }
}
