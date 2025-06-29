#include "StateManager.h"
#include "Game.h"

namespace FishGame
{
    StateManager::StateManager(Game& game)
        : m_game(game)
        , m_stateStack()
        , m_pendingList()
        , m_stateFactories()
    {
        m_stateStack.reserve(10);
        m_pendingList.reserve(10);
        m_stateFactories.reserve(10);
    }

    void StateManager::pushState(StateID id)
    {
        m_pendingList.emplace_back(StateAction::Push, id);
    }

    void StateManager::popState()
    {
        m_pendingList.emplace_back(StateAction::Pop, StateID::None);
    }

    void StateManager::clearStates()
    {
        m_pendingList.emplace_back(StateAction::Clear, StateID::None);
    }

    void StateManager::applyPendingChanges()
    {
        for (const auto& pending : m_pendingList)
        {
            const auto& action = pending.first;
            StateID stateID = pending.second;

            switch (action)
            {
            case StateAction::Push:
            {
                auto newState = createState(stateID);
                newState->onActivate();
                m_stateStack.push_back(std::move(newState));
            }
            break;

            case StateAction::Pop:
                if (!m_stateStack.empty())
                {
                    m_stateStack.back()->onDeactivate();
                    m_stateStack.pop_back();

                    if (!m_stateStack.empty())
                    {
                        m_stateStack.back()->onActivate();
                    }
                }
                break;

            case StateAction::Clear:
                while (!m_stateStack.empty())
                {
                    m_stateStack.back()->onDeactivate();
                    m_stateStack.pop_back();
                }
                break;
            }
        }

        m_pendingList.clear();
    }

    StateManager::StatePtr StateManager::createState(StateID id)
    {
        auto found = m_stateFactories.find(id);
        if (found == m_stateFactories.end())
        {
            throw StateNotFoundException(
                "State factory not found for StateID: " +
                std::to_string(static_cast<int>(id)));
        }

        return found->second();
    }
}
