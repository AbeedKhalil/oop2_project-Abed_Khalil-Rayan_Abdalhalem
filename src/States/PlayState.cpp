#include "PlayState.h"
#include "Game.h"
#include "CollisionDetector.h"
#include "CollisionSystem.h"
#include "PlayLogic.h"
#include "Fish.h"
#include "ExtendedPowerUps.h"
#include "SpawnSystem.h"
#include "EnvironmentController.h"
#include "SpawnController.h"
#include "HUDController.h"
#include "GameOverState.h"
#include "StageIntroState.h"
#include "StageSummaryState.h"
#include "MusicPlayer.h"
#include "Utils/HighScoreIO.h"
#include "GameConstants.h"
#include <execution>
#include <sstream>
#include <iomanip>

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
        , m_systems()
        , m_growthMeter(nullptr)
        , m_frenzySystem(nullptr)
        , m_powerUpManager(nullptr)
        , m_scoreSystem(nullptr)
        , m_bonusItemManager(nullptr)
        , m_oysterManager(nullptr)
        , m_gameState()
        , m_bonusStageTriggered(false)
        , m_returningFromBonusStage(false)
        , m_savedLevel(1)
        , m_particleSystem(std::make_unique<ParticleSystem>())
        , m_collisionSystem(nullptr)
        , m_randomEngine(std::random_device{}())
        , m_angleDist(0.0f, 360.0f)
        , m_speedDist(Constants::MIN_PARTICLE_SPEED, Constants::MAX_PARTICLE_SPEED)
        , m_environmentSystem(std::make_unique<EnvironmentSystem>())
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

        // Initialize controllers
        m_hudController = std::make_unique<HUDController>(font, window.getSize());
        m_environmentController = std::make_unique<EnvironmentController>(
            *m_environmentSystem, *m_player, m_entities, getGame().getSoundPlayer());
        m_spawnController = std::make_unique<SpawnController>(
            *m_fishSpawner, *m_spawnSystem, *m_bonusItemManager,
            m_entities, m_bonusItems, m_hazards);

        // Configure initial state
        m_fishSpawner->setLevel(m_gameState.currentLevel);

        m_collisionSystem = std::make_unique<CollisionSystem>(
            *m_particleSystem,
            *m_scoreSystem,
            *m_frenzySystem,
            *m_powerUpManager,
            m_levelCounts,
            getGame().getSoundPlayer(),
            m_environmentController->stunnedRef(),
            m_environmentController->stunTimerRef(),
            m_environmentController->controlReverseTimerRef(),
            m_gameState.playerLives,
            [this]() { handlePlayerDeath(); },
            [this]() { if (m_environmentController) m_environmentController->applyFreeze(); },
            [this]() { if (m_environmentController) m_environmentController->reverseControls(); }
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

void PlayState::updateGameplay(sf::Time deltaTime)
{
        m_gameState.levelTime += deltaTime;

        updateRespawn(deltaTime);
        if (m_environmentController)
            m_environmentController->update(deltaTime);
        updateSystems(deltaTime);
        updateGameState(deltaTime);
        updateEntities(deltaTime);
        if (m_spawnController)
            m_spawnController->update(deltaTime, m_gameState.currentLevel);

        m_collisionSystem->process(*m_player, m_entities, m_bonusItems, m_hazards,
            m_oysterManager, m_gameState.currentLevel);

        auto active = m_powerUpManager->getActivePowerUps();
        if (m_environmentController && m_hudController)
            m_hudController->update(deltaTime,
                m_scoreSystem->getCurrentScore(),
                m_gameState.playerLives,
                m_gameState.currentLevel,
                m_scoreSystem->getChainBonus(),
                active,
                m_environmentController->isPlayerFrozen(), m_environmentController->getFreezeTimer(),
                m_environmentController->hasControlsReversed(), m_environmentController->getControlReverseTimer(),
                m_environmentController->isPlayerStunned(), m_environmentController->getStunTimer());
    updateCamera();
}

void PlayState::updateRespawn(sf::Time deltaTime)
{
    if (m_musicResumePending)
    {
        m_musicResumeTimer -= deltaTime;
        if (m_musicResumeTimer <= sf::Time::Zero)
        {
            m_musicResumePending = false;
            getGame().getMusicPlayer().play(musicForLevel(m_gameState.currentLevel), true);
        }
    }

    if (m_respawnPending)
    {
        m_respawnTimer -= deltaTime;
        if (m_respawnTimer <= sf::Time::Zero)
        {
            m_respawnPending = false;
            m_player->respawn();
            m_camera.unfreeze();
            createParticleEffect(m_player->getPosition(), Constants::RESPAWN_PARTICLE_COLOR);
        }
    }
}


void PlayState::updateGameState(sf::Time deltaTime)
{
    checkBonusStage();

    if (m_gameState.gameWon)
    {
        m_gameState.winTimer += deltaTime;
        bool timerExpired = m_gameState.winTimer >= Constants::WIN_SEQUENCE_DURATION;
        bool noEnemies = m_gameState.enemiesFleeing && areAllEnemiesGone();

        if (timerExpired || noEnemies)
        {
            m_gameState.enemiesFleeing = false;
            m_gameState.levelComplete = true;
            advanceLevel();
        }

        if (m_gameState.levelComplete)
            return;
    }
    else if (!m_gameState.levelComplete)
    {
        checkWinCondition();
    }
}


    void PlayState::updateSystems(sf::Time deltaTime)
    {
        // Update core systems
        m_frenzySystem->update(deltaTime);
        m_powerUpManager->update(deltaTime);
        m_scoreSystem->update(deltaTime);
        m_growthMeter->update(deltaTime);
        if (m_gameState.currentLevel >= 2)
            m_oysterManager->update(deltaTime);

        // Update schooling system
        m_schoolingSystem->update(deltaTime);

        // Periodically extract fish from schools
        static sf::Time extractTimer = sf::Time::Zero;
        extractTimer += deltaTime;

        if (extractTimer >= Constants::SCHOOL_EXTRACT_INTERVAL)
        {
            extractTimer = sf::Time::Zero;

            auto schoolFish = m_schoolingSystem->extractAllFish();
            std::move(schoolFish.begin(), schoolFish.end(), std::back_inserter(m_entities));
        }

        // Update player
        if (!m_environmentController || !m_environmentController->isPlayerStunned())
        {
            m_player->update(deltaTime);
        }
    }

    void PlayState::updateEntities(sf::Time deltaTime)
    {
        StateUtils::updateEntities(m_entities, deltaTime);
        StateUtils::updateEntities(m_bonusItems, deltaTime);
        StateUtils::updateEntities(m_hazards, deltaTime);

        // Apply specific AI updates
    EntityUtils::forEachAlive(m_entities, [this, deltaTime](Entity& entity) {
        if (auto* fish = dynamic_cast<Fish*>(&entity))
        {
            if (!fish->isStunned())
            {
                fish->updateAI(m_entities, m_player.get(), deltaTime);
            }
        }
    });

    m_particleSystem->update(deltaTime);

    EntityUtils::removeDeadEntities(m_entities);
    EntityUtils::removeDeadEntities(m_hazards);

    m_bonusItems.erase(
        std::remove_if(m_bonusItems.begin(), m_bonusItems.end(),
            [](const auto& item) {
                return !item || !item->isAlive() || item->hasExpired();
            }),
        m_bonusItems.end());
}


    void PlayState::handlePowerUpCollision(PowerUp& powerUp)
    {
        switch (powerUp.getPowerUpType())
        {
        case PowerUpType::ScoreDoubler:
            m_powerUpManager->activatePowerUp(powerUp.getPowerUpType(), powerUp.getDuration());
            createParticleEffect(powerUp.getPosition(), Constants::SCORE_DOUBLER_COLOR);
            break;

        case PowerUpType::FrenzyStarter:
            m_frenzySystem->forceFrenzy();
            createParticleEffect(powerUp.getPosition(), Constants::FRENZY_STARTER_COLOR);
            break;

        case PowerUpType::SpeedBoost:
            m_powerUpManager->activatePowerUp(powerUp.getPowerUpType(), powerUp.getDuration());
            m_player->applySpeedBoost(m_powerUpManager->getSpeedMultiplier(), powerUp.getDuration());
            getGame().getSoundPlayer().play(SoundEffectID::SpeedStart);
            createParticleEffect(powerUp.getPosition(), Constants::SPEED_BOOST_COLOR);
            break;

        case PowerUpType::Freeze:
            m_powerUpManager->activatePowerUp(powerUp.getPowerUpType(), powerUp.getDuration());
            if (m_environmentController)
                m_environmentController->applyFreeze();
            createParticleEffect(powerUp.getPosition(), sf::Color::Cyan, 20);
            break;

        case PowerUpType::ExtraLife:
            m_gameState.playerLives++;
            getGame().getSoundPlayer().play(SoundEffectID::LifePowerup);
            createParticleEffect(powerUp.getPosition(), sf::Color::Green, 15);
            break;
        case PowerUpType::AddTime:
            // No immediate effect handled here
            break;
        }
    }

    void PlayState::handleOysterCollision(PermanentOyster* oyster)
    {
        if (oyster->canDamagePlayer() && !m_player->isInvulnerable())
        {
            m_player->takeDamage();
            handlePlayerDeath();
            createParticleEffect(m_player->getPosition(), Constants::DAMAGE_PARTICLE_COLOR);
        }
        else if (oyster->canBeEaten())
        {
            oyster->onCollect();
            getGame().getSoundPlayer().play(SoundEffectID::OysterPearl);

            int points = oyster->hasBlackPearl()
                ? Constants::BLACK_OYSTER_POINTS
                : Constants::WHITE_OYSTER_POINTS;

            m_player->addPoints(points);
            m_player->grow(oyster->getGrowthPoints());

            int frenzyMultiplier = m_frenzySystem->getMultiplier();
            float powerUpMultiplier = m_powerUpManager->getScoreMultiplier();

            m_scoreSystem->addScore(ScoreEventType::BonusCollected, oyster->getPoints(),
                oyster->getPosition(), frenzyMultiplier, powerUpMultiplier);

            createParticleEffect(oyster->getPosition(),
                oyster->hasBlackPearl() ? Constants::BLACK_PEARL_COLOR : Constants::WHITE_PEARL_COLOR);
        }
    }

    void PlayState::checkWinCondition()
    {
        if (m_player->getPoints() >= Constants::POINTS_TO_WIN)
        {
            triggerWinSequence();
        }
    }

    void PlayState::triggerWinSequence()
    {
        getGame().getMusicPlayer().play(MusicID::StageCleared, false);
        m_gameState.gameWon = true;
        m_gameState.enemiesFleeing = true;
        m_gameState.winTimer = sf::Time::Zero;

        makeAllEnemiesFlee();
        showMessage("LEVEL COMPLETE!\n\nEat the fleeing fish for bonus points!");

        m_fishSpawner->setLevel(-1);
        m_bonusItemManager->setStarfishEnabled(false);
        m_bonusItemManager->setPowerUpsEnabled(false);
    }

    void PlayState::makeAllEnemiesFlee()
    {
        EntityUtils::forEachAlive(m_entities, [](Entity& entity) {
            if (auto* fish = dynamic_cast<Fish*>(&entity))
            {
                fish->startFleeing();
            }
        });
    }

    bool PlayState::areAllEnemiesGone() const
    {
        return std::none_of(m_entities.begin(), m_entities.end(),
            [](const auto& entity) {
                return entity->isAlive() && dynamic_cast<const Fish*>(entity.get()) != nullptr;
            });
    }

    void PlayState::handlePlayerDeath()
    {
        if (m_player->isInvulnerable())
            return;

        // Freeze camera at death position
        m_camera.freeze(m_player->getPosition());

        m_gameState.playerLives--;
        getGame().getMusicPlayer().play(MusicID::PlayerDies, false);
        m_musicResumePending = m_gameState.playerLives > 0;
        if (m_musicResumePending)
            m_musicResumeTimer = sf::seconds(2.0f);
        m_player->die();

        if (m_gameState.playerLives <= 0)
        {
            gameOver();
        }
        else
        {
            m_respawnPending = true;
            m_respawnTimer = Constants::RESPAWN_DELAY;
        }
    }

    void PlayState::advanceLevel()
    {
        int levelScore = m_scoreSystem->getCurrentScore();
        const auto& fishCounts = m_scoreSystem->getFishCounts();
        bool triggerBonus = (m_gameState.currentLevel % 3 == 0);
        StageSummaryState::configure(m_gameState.currentLevel + 1,
                                     levelScore, fishCounts, triggerBonus);
        m_levelCounts.clear();

        m_gameState.currentLevel++;
        m_gameState.totalScore += levelScore;

        updateBackground(m_gameState.currentLevel);

        if (m_gameState.currentLevel % 3 == 0)
        {
            EnvironmentType newEnv = static_cast<EnvironmentType>(
                (static_cast<int>(m_environmentSystem->getCurrentEnvironment()) + 1) % 3);
            m_environmentSystem->setEnvironment(newEnv);
        }

        m_environmentSystem->setRandomTimeOfDay();

        resetLevel();
        updateLevelDifficulty();

        if (m_hudController)
            m_hudController->getSystem().clearMessage();
        m_bonusStageTriggered = false;

        StageIntroState::configure(m_gameState.currentLevel, false);
        deferAction([this]() { requestStackPush(StateID::StageSummary); });
    }

    void PlayState::resetLevel()
    {
        m_player->fullReset();

        // Start player in the middle of the world
        m_player->setPosition(m_camera.getWorldSize() * 0.5f);
        m_camera.getView().setCenter(m_player->getPosition());

        m_gameState.levelComplete = false;
        m_gameState.gameWon = false;
        m_gameState.enemiesFleeing = false;
        m_gameState.levelTime = sf::Time::Zero;

        m_entities.clear();
        m_bonusItems.clear();
        m_hazards.clear();
        m_particleSystem->clear();

        m_scoreSystem->reset();
        m_frenzySystem->reset();
        m_powerUpManager->reset();
        m_growthMeter->reset();
        m_oysterManager->resetAll();

        if (m_environmentController)
            m_environmentController->reset();

        m_bonusItemManager->setStarfishEnabled(true);
        m_bonusItemManager->setPowerUpsEnabled(true);
    }

    void PlayState::gameOver()
    {
        GameStats& stats = GameStats::getInstance();
        stats.finalScore = m_gameState.totalScore;
        stats.levelReached = m_gameState.currentLevel;
        stats.survivalTime = m_gameState.levelTime.asSeconds();
        stats.newHighScore = stats.finalScore > stats.highScore;
        if (stats.newHighScore)
            stats.highScore = stats.finalScore;

        addHighScore("highscores.txt", {stats.playerName, stats.finalScore});

        requestStackClear();
        requestStackPush(StateID::GameOver);
    }

    void PlayState::updateLevelDifficulty()
    {
        m_bonusItemManager->setLevel(m_gameState.currentLevel);

        SpecialFishConfig config;
        float levelMultiplier = 1.0f + (m_gameState.currentLevel - 1) * Constants::DIFFICULTY_INCREMENT;

        config.barracudaSpawnRate = Constants::BARRACUDA_SPAWN_RATE * levelMultiplier;
        config.pufferfishSpawnRate = Constants::PUFFERFISH_SPAWN_RATE * levelMultiplier;
        config.angelfishSpawnRate = Constants::ANGELFISH_SPAWN_RATE * levelMultiplier;
        config.poisonFishSpawnRate = Constants::POISONFISH_SPAWN_RATE * levelMultiplier;
        config.schoolSpawnChance = std::min(Constants::MAX_SCHOOL_SPAWN_CHANCE,
            Constants::SCHOOL_SPAWN_CHANCE * levelMultiplier);

        m_fishSpawner->setSpecialFishConfig(config);
        m_fishSpawner->setLevel(m_gameState.currentLevel);
    }

    void PlayState::checkBonusStage()
    {
        if (!m_bonusStageTriggered && m_gameState.levelComplete && !m_returningFromBonusStage)
        {
            if (m_gameState.currentLevel % 3 == 0)
            {
                m_bonusStageTriggered = true;
                m_savedLevel = m_gameState.currentLevel;

                BonusStageType bonusType = static_cast<BonusStageType>(
                    std::uniform_int_distribution<int>(0, 2)(m_randomEngine));

                deferAction([this, bonusType]() {
                    m_returningFromBonusStage = true;
                    auto& cfg = BonusStageConfig::getInstance();
                    cfg.type = bonusType;
                    cfg.playerLevel = m_savedLevel;
                    StageIntroState::configure(0, true, StateID::BonusStage);
                    requestStackPush(StateID::StageIntro);
                    });
            }
        }
    }

    void PlayState::createParticleEffect(const sf::Vector2f& position, const sf::Color& color, int count)
    {
        m_particleSystem->createEffect(position, color, count);
    }

    void PlayState::updateCamera()
    {
        if (!m_player)
            return;

        m_camera.update(m_player->getPosition());
    }

    void PlayState::showMessage(const std::string& message)
    {
        if (m_hudController)
            m_hudController->showMessage(message);
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
        if (m_hudController)
            window.draw(m_hudController->getSystem());

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

            resetLevel();
            updateLevelDifficulty();
            updateBackground(m_gameState.currentLevel);

            // Mouse control disabled

            if (m_hudController)
                m_hudController->getSystem().clearMessage();
            m_initialized = true;
            getGame().getMusicPlayer().play(musicForLevel(m_gameState.currentLevel), true);
        }
        else if (!m_initialized)
        {
            resetLevel();
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
