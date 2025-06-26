#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <functional>
#include <type_traits>

namespace FishGame
{
    // Forward declarations
    class Game; 

    // State identifiers
    enum class StateID
    {
        None,
        Intro,
        StageIntro,
        StageSummary,
        Menu,
        Play,
        Pause,
        GameOver,
		GameOptions,
        BonusStage
    };

    // State stack actions
    enum class StateAction
    {
        Push,
        Pop,
        Clear
    };

    // Template trait for state validation
    template<typename T>
    struct is_state : std::false_type {};

    // Abstract base class for game states
    class State
    {
    public:
        using StatePtr = std::unique_ptr<State>;
        using EventHandler = std::function<void(const sf::Event&)>;

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

        // Optional virtual functions
        virtual void onActivate() {}
        virtual void onDeactivate() {}

        // Template method for type-safe state queries
        template<typename T>
        bool isType() const
        {
            static_assert(is_state<T>::value, "T must be a State-derived type");
            return dynamic_cast<const T*>(this) != nullptr;
        }

    protected:
        // State management
        void requestStackPush(StateID stateID);
        void requestStackPop();
        void requestStackClear();

        // Accessors
        Game& getGame() { return m_game; }
        const Game& getGame() const { return m_game; }

        // Template utility for deferred actions
        template<typename Func>
        void deferAction(Func&& action)
        {
            m_deferredActions.emplace_back(std::forward<Func>(action));
        }

        void processDeferredActions();

    private:
        Game& m_game;
        std::vector<std::function<void()>> m_deferredActions;
    };

    // Specializations for state trait
    template<> struct is_state<State> : std::true_type {};
}
