#pragma once

#include "IAudioPlayer.h"
#include "ISpriteManager.h"
#include "State.h"
#include "StateManager.h"
#include "Player.h"

namespace FishGame
{
    class Game
    {
    public:
        Game(ISpriteManager& spriteManager, IAudioPlayer& audioPlayer);
        ~Game() = default;

        // Delete copy and move operations - Game is a singleton-like manager
        Game(const Game&) = delete;
        Game& operator=(const Game&) = delete;
        Game(Game&&) = delete;
        Game& operator=(Game&&) = delete;

        void run();

        // Resource accessors
        sf::RenderWindow& getWindow() { return m_window; }
        const sf::RenderWindow& getWindow() const { return m_window; }
        FontHolder& getFonts() { return m_fonts; }
        SpriteManager& getSpriteManager() { return m_spriteManager.getRawManager(); }
        const SpriteManager& getSpriteManager() const { return m_spriteManager.getRawManager(); }
        IAudioPlayer& getAudioPlayer() { return m_audioPlayer; }
        const IAudioPlayer& getAudioPlayer() const { return m_audioPlayer; }

        // State management
        void pushState(StateID id);
        void popState();
        void clearStates();

        // Template method for state queries
        template<typename StateType>
        StateType* getCurrentState() const
        {
            static_assert(std::is_base_of_v<State, StateType>,
                "StateType must be derived from State");

            return m_stateManager.getCurrentState<StateType>();  
        }

    private:
        // Core game loop methods
        void processInput();
        void update(sf::Time deltaTime);
        void render();

        // Initialize graphics system
        void initializeGraphics();

        // State management
        void registerStates();
        void applyPendingStateChanges();
        
        template<typename T>
        void registerState(StateID id)
        {
            m_stateManager.registerState<T>(id);
        }

    private:
        // Window and timing constants from GameConstants.h
        static constexpr unsigned int m_windowWidth = Constants::WINDOW_WIDTH;
        static constexpr unsigned int m_windowHeight = Constants::WINDOW_HEIGHT;
        static constexpr unsigned int m_frameRateLimit = Constants::FRAMERATE_LIMIT;
        static const sf::Time m_timePerFrame;

        // Core systems
        sf::RenderWindow m_window;
        FontHolder m_fonts;
        // State manager
        StateManager m_stateManager;

        ISpriteManager& m_spriteManager;
        IAudioPlayer& m_audioPlayer;

        // Performance tracking
        struct PerformanceMetrics
        {
            sf::Time accumulatedTime = sf::Time::Zero;
            std::size_t frameCount = 0;
            float currentFPS = 0.0f;
        } m_metrics;
    };
}
