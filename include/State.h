#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

namespace FishGame
{
    // Forward declarations
    class Game;

    // State identifiers
    enum class StateID
    {
        None,
        Menu,
        Play,
        Pause,
        GameOver
    };

    // State stack actions
    enum class StateAction
    {
        Push,
        Pop,
        Clear
    };

    // Abstract base class for game states
    class State
    {
    public:
        explicit State(Game& game);
        virtual ~State() = default;

        // Delete copy operations
        State(const State&) = delete;
        State& operator=(const State&) = delete;

        // Allow move operations
        State(State&&) = default;
        State& operator=(State&&) = default;

        // Pure virtual functions
        virtual void handleEvent(const sf::Event& event) = 0;
        virtual bool update(sf::Time deltaTime) = 0;
        virtual void render() = 0;

    protected:
        void requestStackPush(StateID stateID);
        void requestStackPop();
        void requestStackClear();

        Game& getGame() { return m_game; }
        const Game& getGame() const { return m_game; }

    private:
        Game& m_game;
    };
}