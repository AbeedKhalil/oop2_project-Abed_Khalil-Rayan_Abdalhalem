#pragma once

#include "MusicPlayer.h"
#include "State.h"
#include "StateManager.h"
#include "Player.h"

namespace FishGame
{
    class Game
    {
    public:
        Game();
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
        SpriteManager& getSpriteManager() { return *m_spriteManager; }
        MusicPlayer& getMusicPlayer() { return *m_musicPlayer; }
        const MusicPlayer& getMusicPlayer() const { return *m_musicPlayer; }
        SoundPlayer& getSoundPlayer() { return *m_soundPlayer; }
        const SoundPlayer& getSoundPlayer() const { return *m_soundPlayer; }

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
        std::unique_ptr<ResourceHolder<sf::Texture, TextureID>> m_spriteTextures;

        // State manager
        StateManager m_stateManager;

        std::unique_ptr<SpriteManager> m_spriteManager;
        std::unique_ptr<MusicPlayer> m_musicPlayer;
        std::unique_ptr<SoundPlayer> m_soundPlayer;

        // Performance tracking
        struct PerformanceMetrics
        {
            sf::Time accumulatedTime = sf::Time::Zero;
            std::size_t frameCount = 0;
            float currentFPS = 0.0f;
        } m_metrics;
    };
}
