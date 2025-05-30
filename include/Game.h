#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <array>
#include <stack>
#include <functional>
#include "ResourceHolder.h"
#include "State.h"
#include "Player.h"

namespace FishGame
{
    class Game
    {
    public:
        Game();
        ~Game() = default;

        // Delete copy and move operations
        Game(const Game&) = delete;
        Game& operator=(const Game&) = delete;
        Game(Game&&) = delete;
        Game& operator=(Game&&) = delete;

        void run();

        // Resource accessors
        sf::RenderWindow& getWindow() { return m_window; }
        const sf::RenderWindow& getWindow() const { return m_window; }
        TextureHolder& getTextures() { return m_textures; }
        FontHolder& getFonts() { return m_fonts; }

        // State management
        void pushState(StateID id);
        void popState();
        void clearStates();

    private:
        void processInput();
        void update(sf::Time deltaTime);
        void render();
        void registerStates();

        template<typename T>
        void registerState(StateID id);

        std::unique_ptr<State> createState(StateID id);

    private:
        static constexpr unsigned int m_windowWidth = 1920;
        static constexpr unsigned int m_windowHeight = 1080;
        static const sf::Time m_timePerFrame;

        sf::RenderWindow m_window;
        TextureHolder m_textures;
        FontHolder m_fonts;

        std::stack<std::unique_ptr<State>> m_stateStack;
        std::vector<std::pair<StateAction, StateID>> m_pendingList;

        std::map<StateID, std::function<std::unique_ptr<State>()>> m_stateFactories;
    };

    // Template implementation
    template<typename T>
    void Game::registerState(StateID id)
    {
        m_stateFactories[id] = [this]()
            {
                return std::make_unique<T>(*this);
            };
    }
}