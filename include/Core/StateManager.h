#pragma once

#include "State.h"
#include "GameExceptions.h"
#include <unordered_map>
#include <vector>

namespace FishGame
{
    class Game;

    class StateManager
    {
    public:
        using StatePtr = std::unique_ptr<State>;
        using StateFactory = std::function<StatePtr()>;

        explicit StateManager(Game& game);
        ~StateManager() = default;

        StateManager(const StateManager&) = delete;
        StateManager& operator=(const StateManager&) = delete;
        StateManager(StateManager&&) = delete;
        StateManager& operator=(StateManager&&) = delete;

        void pushState(StateID id);
        void popState();
        void clearStates();

        template<typename StateType>
        StateType* getCurrentState() const
        {
            static_assert(std::is_base_of_v<State, StateType>,
                "StateType must be derived from State");

            if (!m_stateStack.empty())
            {
                return dynamic_cast<StateType*>(m_stateStack.back().get());
            }
            return nullptr;
        }

        template<typename T>
        void registerState(StateID id)
        {
            static_assert(std::is_base_of_v<State, T>,
                "T must be derived from State");

            m_stateFactories[id] = [this]() -> StatePtr
            {
                return std::make_unique<T>(m_game);
            };
        }

        void registerState(StateID id, StateFactory factory)
        {
            m_stateFactories[id] = std::move(factory);
        }

        void applyPendingChanges();

        bool empty() const { return m_stateStack.empty(); }
        const auto& getStateStack() const { return m_stateStack; }

    private:
        StatePtr createState(StateID id);

        Game& m_game;
        std::vector<StatePtr> m_stateStack;
        std::vector<std::pair<StateAction, StateID>> m_pendingList;
        std::unordered_map<StateID, StateFactory> m_stateFactories;
    };
}
