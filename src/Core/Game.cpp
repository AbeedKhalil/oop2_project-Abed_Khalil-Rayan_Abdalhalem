#include "Game.h"
#include "IntroState.h"
#include "PlayerNameState.h"
#include "MenuState.h"
#include "HighScoresState.h"
#include "PlayState.h"
#include "GameOverState.h"
#include "GameOptionsState.h"
#include "StageIntroState.h"
#include "StageSummaryState.h"
#include "BonusStageState.h"

namespace FishGame
{
    // Static member initialization using GameConstants
    const sf::Time Game::m_timePerFrame = sf::seconds(1.0f / Constants::FRAMERATE_LIMIT);

    Game::Game()
        : m_window(sf::VideoMode(m_windowWidth, m_windowHeight),
            Constants::GAME_TITLE,
            sf::Style::Close)
        , m_fonts()
        , m_spriteTextures(nullptr)
        , m_stateManager(*this)
        , m_spriteManager(nullptr)
        , m_musicPlayer(std::make_unique<MusicPlayer>())
        , m_soundPlayer(std::make_unique<SoundPlayer>())
        , m_metrics()
    {
        m_window.setFramerateLimit(m_frameRateLimit);

        // Reserve capacity for better performance

        // Load resources
        m_fonts.load(Fonts::Main, "Regular.ttf");

        initializeGraphics();

        registerStates();
        pushState(StateID::Intro);
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
                if (m_stateManager.empty())
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
        m_spriteManager->loadTextures("");

        // Configure sprite scales
        SpriteScaleConfig scaleConfig;
        scaleConfig.small = 0.5f;
        scaleConfig.medium = 0.8f;
        scaleConfig.large = 1.1f;
        m_spriteManager->setScaleConfig(scaleConfig);
    }

    void Game::processInput()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            if (event.type == sf::Event::KeyPressed ||
                event.type == sf::Event::MouseButtonPressed)
            {
                m_window.requestFocus();
            }
            // Process events through active states using STL algorithms
            const auto& stack = m_stateManager.getStateStack();
            std::for_each(stack.rbegin(),
                stack.rend(),
                [&event](const StateManager::StatePtr& state)
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
        const auto& stack = m_stateManager.getStateStack();
        auto updateState = [deltaTime](const StateManager::StatePtr& state) -> bool
            {
                return state->update(deltaTime);
            };

        // Update states until one returns false (blocks updates below)
        std::find_if(stack.rbegin(), stack.rend(),
            [&updateState](const StateManager::StatePtr& state)
            {
                return !updateState(state);
            });

        // Apply any pending state changes
        m_stateManager.applyPendingChanges();
    }

    void Game::render()
    {
        m_window.clear(Constants::OCEAN_BLUE);

        // Render all states from bottom to top using STL
        const auto& stack = m_stateManager.getStateStack();
        std::for_each(stack.begin(), stack.end(),
            [this](const StateManager::StatePtr& state)
            {
                state->render();
            });

        m_window.display();
    }

    void Game::pushState(StateID id)
    {
        m_stateManager.pushState(id);
    }

    void Game::popState()
    {
        m_stateManager.popState();
    }

    void Game::clearStates()
    {
        m_stateManager.clearStates();
    }

    void Game::applyPendingStateChanges()
    {
        m_stateManager.applyPendingChanges();
    }

    void Game::registerStates()
    {
        // Register all game states using template method
        registerState<IntroState>(StateID::Intro);
        registerState<PlayerNameState>(StateID::PlayerName);
        registerState<MenuState>(StateID::Menu);
        registerState<StageIntroState>(StateID::StageIntro);
        registerState<StageSummaryState>(StateID::StageSummary);
        registerState<PlayState>(StateID::Play);
        // Register bonus stage
        m_stateManager.registerState(StateID::BonusStage, [this]() -> std::unique_ptr<State>
            {
                auto& cfg = BonusStageConfig::getInstance();
                return std::make_unique<BonusStageState>(*this, cfg.type, cfg.playerLevel);
            });

        // Register optional states
        registerState<GameOptionsState>(StateID::GameOptions);
        registerState<GameOverState>(StateID::GameOver);
        registerState<HighScoresState>(StateID::HighScores);
    }

}
