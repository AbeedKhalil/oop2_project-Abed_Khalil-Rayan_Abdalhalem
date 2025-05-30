#include "Game.h"
#include "MenuState.h"
#include "PlayState.h"
#include <algorithm>
#include <stdexcept>

namespace FishGame
{
    // Static member initialization
    const sf::Time Game::m_timePerFrame = sf::seconds(1.f / 60.f);

    Game::Game()
        : m_window(sf::VideoMode(m_windowWidth, m_windowHeight), "Feeding Frenzy", sf::Style::Close)
        , m_textures()
        , m_fonts()
        , m_stateStack()
        , m_pendingList()
        , m_stateFactories()
    {
        m_window.setFramerateLimit(60);

        // Load resources
        if (!m_fonts.load(Fonts::Main, "Regular.ttf"))
        {
            throw std::runtime_error("Failed to load font");
        }

        registerStates();
        pushState(StateID::Menu);
    }

    void Game::run()
    {
        sf::Clock clock;
        sf::Time timeSinceLastUpdate = sf::Time::Zero;

        while (m_window.isOpen())
        {
            sf::Time deltaTime = clock.restart();
            timeSinceLastUpdate += deltaTime;

            while (timeSinceLastUpdate > m_timePerFrame)
            {
                timeSinceLastUpdate -= m_timePerFrame;

                processInput();
                update(m_timePerFrame);

                // Check if we need to close
                if (m_stateStack.empty())
                    m_window.close();
            }

            render();
        }
    }

    void Game::processInput()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            if (!m_stateStack.empty())
            {
                m_stateStack.top()->handleEvent(event);
            }

            if (event.type == sf::Event::Closed)
            {
                m_window.close();
            }
        }
    }

    void Game::update(sf::Time deltaTime)
    {
        // Update active states from top to bottom
        for (auto it = m_stateStack.size(); it > 0; --it)
        {
            auto& state = *std::next(m_stateStack.top().get(), -static_cast<int>(it - 1));
            if (!state.update(deltaTime))
                break;
        }

        // Apply pending changes
        for (auto& [action, stateID] : m_pendingList)
        {
            switch (action)
            {
            case StateAction::Push:
                m_stateStack.push(createState(stateID));
                break;

            case StateAction::Pop:
                if (!m_stateStack.empty())
                    m_stateStack.pop();
                break;

            case StateAction::Clear:
                while (!m_stateStack.empty())
                    m_stateStack.pop();
                break;
            }
        }

        m_pendingList.clear();
    }

    void Game::render()
    {
        m_window.clear(sf::Color(0, 100, 150)); // Ocean blue background

        // Render all states from bottom to top
        std::vector<State*> renderOrder;

        // Convert stack to vector for bottom-up rendering
        std::stack<std::unique_ptr<State>> tempStack;
        while (!m_stateStack.empty())
        {
            renderOrder.push_back(m_stateStack.top().get());
            tempStack.push(std::move(m_stateStack.top()));
            m_stateStack.pop();
        }

        // Restore stack
        while (!tempStack.empty())
        {
            m_stateStack.push(std::move(tempStack.top()));
            tempStack.pop();
        }

        // Render in correct order
        std::reverse(renderOrder.begin(), renderOrder.end());
        for (auto* state : renderOrder)
        {
            state->render();
        }

        m_window.display();
    }

    void Game::pushState(StateID id)
    {
        m_pendingList.emplace_back(StateAction::Push, id);
    }

    void Game::popState()
    {
        m_pendingList.emplace_back(StateAction::Pop, StateID::None);
    }

    void Game::clearStates()
    {
        m_pendingList.emplace_back(StateAction::Clear, StateID::None);
    }

    void Game::registerStates()
    {
        registerState<MenuState>(StateID::Menu);
        registerState<PlayState>(StateID::Play);
    }

    std::unique_ptr<State> Game::createState(StateID id)
    {
        auto found = m_stateFactories.find(id);
        if (found == m_stateFactories.end())
        {
            throw std::runtime_error("State factory not found");
        }

        return found->second();
    }
}