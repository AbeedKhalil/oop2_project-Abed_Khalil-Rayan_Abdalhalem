#include "PlayState.h"
#include "Game.h"
#include "CollisionDetector.h"
#include "CollisionSystem.h"
#include "PlayLogic.h"
#include "Fish.h"
#include "ExtendedPowerUps.h"
#include "SpawnSystem.h"
#include "GameOverState.h"
#include "StageIntroState.h"
#include "StageSummaryState.h"
#include "MusicPlayer.h"
#include "GameConstants.h"
#include <algorithm>
#include <execution>
#include <sstream>
#include <iomanip>
#include <numeric>

namespace FishGame
{
    // Select appropriate background music for the given level
    static MusicID musicForLevel(int level);

    PlayState::PlayState(Game& game)
        : State(game)
        , m_player(std::make_unique<Player>())
        , m_fishSpawner(std::make_unique<EnhancedFishSpawner>(getGame().getWindow().getSize(), getGame().getSpriteManager()))
        , m_schoolingSystem(std::make_unique<SchoolingSystem>())
        , m_entities()
        , m_bonusItems()
        , m_hazards()
        , m_environmentSystem(std::make_unique<EnvironmentSystem>())
        , m_systems()
        , m_growthMeter(nullptr)
        , m_frenzySystem(nullptr)
        , m_powerUpManager(nullptr)
        , m_scoreSystem(nullptr)
        , m_bonusItemManager(nullptr)
        , m_oysterManager(nullptr)
        , m_gameState()
        , m_isPlayerFrozen(false)
        , m_hasControlsReversed(false)
        , m_isPlayerStunned(false)
        , m_controlReverseTimer(sf::Time::Zero)
        , m_freezeTimer(sf::Time::Zero)
        , m_stunTimer(sf::Time::Zero)
        , m_hazardSpawnTimer(sf::seconds(m_hazardSpawnInterval))
        , m_extendedPowerUpSpawnTimer(sf::seconds(m_extendedPowerUpInterval))
        , m_bonusStageTriggered(false)
        , m_returningFromBonusStage(false)
        , m_savedLevel(1)
        , m_metrics()
        , m_particleSystem(std::make_unique<ParticleSystem>())
        , m_collisionSystem(nullptr)
        , m_randomEngine(std::random_device{}())
        , m_angleDist(0.0f, 360.0f)
        , m_speedDist(Constants::MIN_PARTICLE_SPEED, Constants::MAX_PARTICLE_SPEED)
        , m_spawnSystem(std::make_unique<SpawnSystem>(
            getGame().getSpriteManager(), m_randomEngine,
            m_gameState.currentLevel, getGame().getFonts().get(Fonts::Main)))
        , m_initialized(false)
        , m_logic(nullptr)
    {
        initializeSystems();
        m_player->setSoundPlayer(&getGame().getSoundPlayer());
        if (m_frenzySystem)
            m_frenzySystem->setSoundPlayer(&getGame().getSoundPlayer());

        // Reserve capacity for containers
        m_entities.reserve(Constants::MAX_ENTITIES);
        m_bonusItems.reserve(Constants::MAX_BONUS_ITEMS);
        m_hazards.reserve(20);

        // Setup background and camera
        auto& window = getGame().getWindow();
        updateBackground(m_gameState.currentLevel);

        const sf::Vector2f windowSize(window.getSize());

        sf::View view = window.getDefaultView();
        view.zoom(Constants::CAMERA_ZOOM_FACTOR);
        view.setCenter(windowSize * 0.5f);
        m_camera = CameraController(view, windowSize);

        m_logic = std::make_unique<PlayLogic>(*this);
    }


    void PlayState::initializeSystems()
    {
        auto& window = getGame().getWindow();
        auto& font = getGame().getFonts().get(Fonts::Main);

        // Create game systems through helper
        m_systems.initialize(font, window.getSize(), getGame().getSpriteManager());

        // Cache raw pointers for convenience
        m_growthMeter = &m_systems.getGrowthMeter();
        m_frenzySystem = &m_systems.getFrenzySystem();
        m_powerUpManager = &m_systems.getPowerUpManager();
        m_scoreSystem = &m_systems.getScoreSystem();
        m_bonusItemManager = &m_systems.getBonusItemManager();
        m_oysterManager = &m_systems.getOysterManager();

        // Initialize environment system
        m_environmentSystem->setEnvironment(EnvironmentType::OpenOcean);
        m_environmentSystem->pauseDayNightCycle();

        // Initialize player with systems
        m_player->setWindowBounds(window.getSize());
        m_player->initializeSystems(m_growthMeter, m_frenzySystem, m_powerUpManager, m_scoreSystem);
        m_player->initializeSprite(getGame().getSpriteManager());

        // Configure spawners
        m_fishSpawner->setSchoolingSystem(m_schoolingSystem.get());

        // Use full initializer list so school spawn chance is correctly set
        SpecialFishConfig specialConfig{
            Constants::BARRACUDA_SPAWN_RATE,
            Constants::PUFFERFISH_SPAWN_RATE,
            Constants::ANGELFISH_SPAWN_RATE,
            Constants::POISONFISH_SPAWN_RATE,
            Constants::SCHOOL_SPAWN_CHANCE
        };
        m_fishSpawner->setSpecialFishConfig(specialConfig);

        // Position UI elements
        float growthMeterX = window.getSize().x - Constants::HUD_MARGIN - 300.0f;
        float growthMeterY = Constants::HUD_MARGIN + 20.0f;
        m_growthMeter->setPosition(growthMeterX, growthMeterY);
        m_frenzySystem->setPosition(window.getSize().x / 2.0f, Constants::FRENZY_Y_POSITION);

        // Initialize HUD system
        m_hudSystem = std::make_unique<HUDSystem>(font, window.getSize());

        // Configure initial state
        m_fishSpawner->setLevel(m_gameState.currentLevel);

        m_collisionSystem = std::make_unique<CollisionSystem>(
            *m_particleSystem,
            *m_scoreSystem,
            *m_frenzySystem,
            *m_powerUpManager,
            m_levelCounts,
            getGame().getSoundPlayer(),
            m_isPlayerStunned,
            m_stunTimer,
            m_controlReverseTimer,
            m_gameState.playerLives,
            [this]() { m_logic->handlePlayerDeath(); },
            [this]() { m_logic->applyFreeze(); },
            [this]() { m_logic->reverseControls(); }
        );

    }


    void PlayState::handleEvent(const sf::Event& event)
    {
        if (m_logic)
            m_logic->handleEvent(event);
    }

bool PlayState::update(sf::Time deltaTime)
{
    if (m_logic)
        return m_logic->update(deltaTime);
    return false;
}

    void PlayState::render()
    {
        auto& window = getGame().getWindow();
        auto defaultView = window.getView();
        window.setView(m_camera.getView());

        window.draw(m_backgroundSprite);
        window.draw(*m_environmentSystem);

        if (m_gameState.currentLevel >= 2)
            m_oysterManager->draw(window);

        StateUtils::renderContainer(m_hazards, window);
        StateUtils::renderContainer(m_entities, window);

        StateUtils::renderContainer(m_bonusItems, window);

        window.draw(*m_player);

        window.draw(*m_particleSystem);

        m_scoreSystem->drawFloatingScores(window);
        
        window.setView(defaultView);
        window.draw(*m_growthMeter);
        window.draw(*m_frenzySystem);



        // Render HUD
        window.draw(*m_hudSystem);

        if (m_gameState.gameWon || m_gameState.levelComplete)
        {
            sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
            overlay.setFillColor(Constants::OVERLAY_COLOR);
            window.draw(overlay);
            // message rendered via HUD system
        }
    }

    void PlayState::onActivate()
    {
        if (m_returningFromBonusStage)
        {
            m_returningFromBonusStage = false;
            m_bonusStageTriggered = false;

            m_gameState.currentLevel = m_savedLevel + 1;
            m_gameState.levelComplete = false;
            m_gameState.gameWon = false;
            m_gameState.enemiesFleeing = false;
            m_gameState.levelTime = sf::Time::Zero;

            m_logic->resetLevel();
            m_logic->updateLevelDifficulty();
            updateBackground(m_gameState.currentLevel);

            // Mouse control disabled

            m_hudSystem->clearMessage();
            m_initialized = true;
            getGame().getMusicPlayer().play(musicForLevel(m_gameState.currentLevel), true);
        }
        else if (!m_initialized)
        {
            m_logic->resetLevel();
            m_gameState.currentLevel = 1;
            m_gameState.playerLives = Constants::INITIAL_LIVES;
            m_gameState.totalScore = 0;
            m_bonusStageTriggered = false;
            m_returningFromBonusStage = false;
            m_savedLevel = 1;
            m_initialized = true;
            updateBackground(m_gameState.currentLevel);
            getGame().getMusicPlayer().play(musicForLevel(m_gameState.currentLevel), true);
        }
        else
        {
            // Resume in-game music after pause or stage intro
            getGame().getMusicPlayer().play(musicForLevel(m_gameState.currentLevel), true);
        }

        // Ensure camera starts centered on the player
        updateCamera();
    }

    void PlayState::onDeactivate()
    {
        // Show mouse cursor again when leaving play state
        getGame().getWindow().setMouseCursorVisible(true);
    }

    static MusicID musicForLevel(int level)
    {
        int index = ((level - 1) / 3) % 3;
        switch (index)
        {
        case 0: return MusicID::InGame1;
        case 1: return MusicID::InGame2;
        default: return MusicID::InGame3;
        }
    }

    void PlayState::updateBackground(int level)
    {
        static const TextureID backgrounds[] = {
            TextureID::Background1,
            TextureID::Background2,
            TextureID::Background3,
            TextureID::Background4,
            TextureID::Background5
        };

        int index = ((level - 1) / 2) % 5;
        TextureID id = backgrounds[index];

        auto& manager = getGame().getSpriteManager();
        m_backgroundSprite.setTexture(manager.getTexture(id));

        auto windowSize = getGame().getWindow().getSize();
        auto texSize = m_backgroundSprite.getTexture()->getSize();
        m_backgroundSprite.setScale(
            static_cast<float>(windowSize.x) / texSize.x,
            static_cast<float>(windowSize.y) / texSize.y);
    }
}
