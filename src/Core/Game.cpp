#include "Game.h"
#include "SpriteManager.h" 
#include "MenuState.h"
#include "PlayState.h"
#include "GameOverState.h"
#include "PauseState.h"
#include <algorithm>
#include <stdexcept>
#include <numeric>
#include <execution>

namespace FishGame
{
    // Static member initialization using GameConstants
    const sf::Time Game::m_timePerFrame = sf::seconds(1.0f / Constants::FRAMERATE_LIMIT);

    Game::Game()
        : m_window(sf::VideoMode(m_windowWidth, m_windowHeight),
            Constants::GAME_TITLE,
            sf::Style::Close)
        , m_textures()
        , m_fonts()
        , m_spriteTextures(nullptr)
        , m_stateStack()
        , m_pendingList()
        , m_stateFactories()
        , m_metrics()
        , m_spriteManager(nullptr)
    {
        m_window.setFramerateLimit(m_frameRateLimit);

        // Reserve capacity for better performance
        m_pendingList.reserve(10);

        // Load resources
        if (!m_fonts.load(Fonts::Main, "Regular.ttf"))
        {
            throw std::runtime_error("Failed to load font: Regular.ttf");
        }

        initializeGraphics();

        registerStates();
        pushState(StateID::Menu);
    }

    void Game::run()
    {
        sf::Clock clock;
        sf::Time timeSinceLastUpdate = sf::Time::Zero;

        while (m_window.isOpen())
        {
            const sf::Time deltaTime = clock.restart();
            timeSinceLastUpdate += deltaTime;

            // Update performance metrics
            m_metrics.accumulatedTime += deltaTime;
            ++m_metrics.frameCount;

            if (m_metrics.accumulatedTime >= sf::seconds(1.0f))
            {
                m_metrics.currentFPS = static_cast<float>(m_metrics.frameCount) /
                    m_metrics.accumulatedTime.asSeconds();
                m_metrics.frameCount = 0;
                m_metrics.accumulatedTime = sf::Time::Zero;
            }

            // Fixed timestep with interpolation
            while (timeSinceLastUpdate > m_timePerFrame)
            {
                timeSinceLastUpdate -= m_timePerFrame;

                processInput();
                update(m_timePerFrame);

                // Check if we need to close
                if (m_stateStack.empty())
                {
                    m_window.close();
                }
            }

            render();
        }
    }

    void Game::initializeGraphics()
    {
        // Create sprite manager with persistent texture holder
        m_spriteTextures = std::make_unique<ResourceHolder<sf::Texture, TextureID>>();
        m_spriteManager = std::make_unique<SpriteManager>(*m_spriteTextures);

        // Load all sprite textures
        if (!m_spriteManager->loadTextures(""))
        {
			//throw std::runtime_error("Failed to load sprite textures");
        }

        // Configure sprite scales
        SpriteScaleConfig scaleConfig;
        scaleConfig.small = 0.5f;    // Adjust based on your sprite sizes
        scaleConfig.medium = 0.75f;
        scaleConfig.large = 1.0f;
        m_spriteManager->setScaleConfig(scaleConfig);
    }

    void Game::processInput()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            // Process events through active states using STL algorithms
            std::for_each(m_stateStack._Get_container().rbegin(),
                m_stateStack._Get_container().rend(),
                [&event](const StatePtr& state)
                {
                    state->handleEvent(event);
                });

            if (event.type == sf::Event::Closed)
            {
                m_window.close();
            }
        }
    }

    void Game::update(sf::Time deltaTime)
    {
        // Update active states from top to bottom using STL
        const auto& container = m_stateStack._Get_container();

        auto updateState = [deltaTime](const StatePtr& state) -> bool
            {
                return state->update(deltaTime);
            };

        // Update states until one returns false (blocks updates below)
        auto it = std::find_if(container.rbegin(), container.rend(),
            [&updateState](const StatePtr& state)
            {
                return !updateState(state);
            });

        // Apply any pending state changes
        applyPendingStateChanges();
    }

    void Game::render()
    {
        m_window.clear(Constants::OCEAN_BLUE);

        // Render all states from bottom to top using STL
        const auto& container = m_stateStack._Get_container();
        std::for_each(container.begin(), container.end(),
            [this](const StatePtr& state)
            {
                state->render();
            });

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

    void Game::applyPendingStateChanges()
    {
        // Process all pending state changes using STL
        std::for_each(m_pendingList.begin(), m_pendingList.end(),
            [this](const auto& pending)
            {
                const auto& [action, stateID] = pending;

                switch (action)
                {
                case StateAction::Push:
                {
                    auto newState = createState(stateID);
                    newState->onActivate();
                    m_stateStack.push(std::move(newState));
                }
                break;

                case StateAction::Pop:
                    if (!m_stateStack.empty())
                    {
                        m_stateStack.top()->onDeactivate();
                        m_stateStack.pop();

                        if (!m_stateStack.empty())
                        {
                            m_stateStack.top()->onActivate();
                        }
                    }
                    break;

                case StateAction::Clear:
                    // Clear stack using STL container operations
                    while (!m_stateStack.empty())
                    {
                        m_stateStack.top()->onDeactivate();
                        m_stateStack.pop();
                    }
                    break;
                }
            });

        m_pendingList.clear();
    }

    void Game::registerStates()
    {
        // Register all game states using template method
        registerState<MenuState>(StateID::Menu);
        registerState<PlayState>(StateID::Play);
        // Register bonus stage
        m_stateFactories[StateID::BonusStage] = [this]() -> std::unique_ptr<State>
            {
                // Default to treasure hunt, level 1
                return std::make_unique<BonusStageState>(*this, BonusStageType::TreasureHunt, 1);
            };

        // TODO: Register additional states as they are implemented
        registerState<PauseState>(StateID::Pause);
        registerState<GameOverState>(StateID::GameOver);
        // registerState<SettingsState>(StateID::Settings);
        // registerState<CreditsState>(StateID::Credits);
    }

    std::unique_ptr<State> Game::createState(StateID id)
    {
        auto found = m_stateFactories.find(id);
        if (found == m_stateFactories.end())
        {
            throw std::runtime_error("State factory not found for StateID: " +
                std::to_string(static_cast<int>(id)));
        }

        return found->second();
    }
}