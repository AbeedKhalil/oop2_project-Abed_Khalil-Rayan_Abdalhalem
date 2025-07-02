#include "PlayLogic.h"
#include "PlayState.h"
#include "Game.h"
#include "StageIntroState.h"
#include "StageSummaryState.h"
#include "GameOverState.h"
#include "BonusStageState.h"
#include <algorithm>
#include <numeric>
#include <random>

extern FishGame::MusicID musicForLevel(int level);

namespace FishGame {

PlayLogic::PlayLogic(PlayState& state) : m_state(state) {}

void PlayLogic::handleEvent(const sf::Event& event)
{
    if (m_state.m_isPlayerStunned || m_state.getGame().getCurrentState<StageIntroState>())
        return;

    m_state.m_inputHandler.setReversed(m_state.m_hasControlsReversed);
    m_state.m_inputHandler.processEvent(event, [this](const sf::Event& processedEvent)
    {
        switch (processedEvent.type)
        {
        case sf::Event::KeyPressed:
            switch (processedEvent.key.code)
            {
            case sf::Keyboard::Escape:
                m_state.deferAction([this]() {
                    m_state.requestStackPop();
                    m_state.requestStackPush(StateID::Menu);
                });
                break;
            case sf::Keyboard::P:
                m_state.deferAction([this]() {
                    StageIntroState::configure(m_state.m_gameState.currentLevel, false);
                    m_state.requestStackPush(StateID::StageIntro);
                });
                break;
            case sf::Keyboard::W:
            case sf::Keyboard::S:
            case sf::Keyboard::A:
            case sf::Keyboard::D:
            case sf::Keyboard::Up:
            case sf::Keyboard::Down:
            case sf::Keyboard::Left:
            case sf::Keyboard::Right:
                break;
            default:
                break;
            }
            break;
        case sf::Event::MouseMoved:
            break;
        case sf::Event::MouseButtonPressed:
            break;
        default:
            break;
        }
    });
}

bool PlayLogic::update(sf::Time deltaTime)
{
    updatePerformanceMetrics(deltaTime);
    updateGameplay(deltaTime);
    m_state.processDeferredActions();
    return false;
}

void PlayLogic::updateGameplay(sf::Time deltaTime)
{
    m_state.m_gameState.levelTime += deltaTime;

    updateRespawn(deltaTime);
    updateEnvironment(deltaTime);
    updateGameState(deltaTime);
    updateEntities(deltaTime);
    updateSpawning(deltaTime);

    m_state.m_collisionSystem->process(*m_state.m_player,
        m_state.m_entities, m_state.m_bonusItems, m_state.m_hazards,
        m_state.m_oysterManager, m_state.m_gameState.currentLevel);

    updateHUD();
    updateCamera();
}

void PlayLogic::updateRespawn(sf::Time deltaTime)
{
    if (m_state.m_musicResumePending)
    {
        m_state.m_musicResumeTimer -= deltaTime;
        if (m_state.m_musicResumeTimer <= sf::Time::Zero)
        {
            m_state.m_musicResumePending = false;
            m_state.getGame().getMusicPlayer().play(
                musicForLevel(m_state.m_gameState.currentLevel), true);
        }
    }

    if (m_state.m_respawnPending)
    {
        m_state.m_respawnTimer -= deltaTime;
        if (m_state.m_respawnTimer <= sf::Time::Zero)
        {
            m_state.m_respawnPending = false;
            m_state.m_player->respawn();
            m_state.m_camera.unfreeze();
            createParticleEffect(m_state.m_player->getPosition(),
                                Constants::RESPAWN_PARTICLE_COLOR);
        }
    }
}

void PlayLogic::updateEnvironment(sf::Time deltaTime)
{
    m_state.m_environmentSystem->update(deltaTime);
    updateSystems(deltaTime);
    updateEffectTimers(deltaTime);
    applyEnvironmentalForces(deltaTime);
}

void PlayLogic::updateGameState(sf::Time deltaTime)
{
    checkBonusStage();

    if (m_state.m_gameState.gameWon)
    {
        m_state.m_gameState.winTimer += deltaTime;
        bool timerExpired = m_state.m_gameState.winTimer >= Constants::WIN_SEQUENCE_DURATION;
        bool noEnemies = m_state.m_gameState.enemiesFleeing && areAllEnemiesGone();

        if (timerExpired || noEnemies)
        {
            m_state.m_gameState.enemiesFleeing = false;
            m_state.m_gameState.levelComplete = true;
            advanceLevel();
        }

        if (m_state.m_gameState.levelComplete)
            return;
    }
    else if (!m_state.m_gameState.levelComplete)
    {
        checkWinCondition();
    }
}

void PlayLogic::updateSpawning(sf::Time deltaTime)
{
    if (m_state.m_gameState.gameWon)
        return;

    m_state.m_fishSpawner->update(deltaTime, m_state.m_gameState.currentLevel);
    auto& spawnedFish = m_state.m_fishSpawner->getSpawnedFish();
    std::move(spawnedFish.begin(), spawnedFish.end(), std::back_inserter(m_state.m_entities));
    spawnedFish.clear();

    if (m_state.m_hazardSpawnTimer.update(deltaTime))
    {
        if (auto hazard = m_state.m_spawnSystem->spawnRandomHazard())
            m_state.m_hazards.push_back(std::move(hazard));
    }

    if (m_state.m_extendedPowerUpSpawnTimer.update(deltaTime))
    {
        if (auto powerUp = m_state.m_spawnSystem->spawnRandomPowerUp())
            m_state.m_bonusItems.push_back(std::move(powerUp));
    }

    m_state.m_bonusItemManager->update(deltaTime);
    auto newItems = m_state.m_bonusItemManager->collectSpawnedItems();
    std::move(newItems.begin(), newItems.end(), std::back_inserter(m_state.m_bonusItems));
}

void PlayLogic::updateSystems(sf::Time deltaTime)
{
    m_state.m_frenzySystem->update(deltaTime);
    m_state.m_powerUpManager->update(deltaTime);
    m_state.m_scoreSystem->update(deltaTime);
    m_state.m_growthMeter->update(deltaTime);
    if (m_state.m_gameState.currentLevel >= 2)
        m_state.m_oysterManager->update(deltaTime);

    m_state.m_schoolingSystem->update(deltaTime);

    static sf::Time extractTimer = sf::Time::Zero;
    extractTimer += deltaTime;
    if (extractTimer >= Constants::SCHOOL_EXTRACT_INTERVAL)
    {
        extractTimer = sf::Time::Zero;
        auto schoolFish = m_state.m_schoolingSystem->extractAllFish();
        std::move(schoolFish.begin(), schoolFish.end(), std::back_inserter(m_state.m_entities));
    }

    if (!m_state.m_isPlayerStunned)
        m_state.m_player->update(deltaTime);
}

void PlayLogic::updateEntities(sf::Time deltaTime)
{
    StateUtils::updateEntities(m_state.m_entities, deltaTime);
    StateUtils::updateEntities(m_state.m_bonusItems, deltaTime);
    StateUtils::updateEntities(m_state.m_hazards, deltaTime);

    EntityUtils::forEachAlive(m_state.m_entities, [this, deltaTime](Entity& entity) {
        if (auto* fish = dynamic_cast<Fish*>(&entity))
        {
            if (!fish->isStunned())
                fish->updateAI(m_state.m_entities, m_state.m_player.get(), deltaTime);
        }
    });

    m_state.m_particleSystem->update(deltaTime);

    EntityUtils::removeDeadEntities(m_state.m_entities);
    EntityUtils::removeDeadEntities(m_state.m_hazards);

    m_state.m_bonusItems.erase(
        std::remove_if(m_state.m_bonusItems.begin(), m_state.m_bonusItems.end(),
            [](const auto& item) { return !item || !item->isAlive() || item->hasExpired(); }),
        m_state.m_bonusItems.end());
}

void PlayLogic::updateEffectTimers(sf::Time deltaTime)
{
    if (m_state.m_isPlayerFrozen)
    {
        m_state.m_freezeTimer -= deltaTime;
        if (m_state.m_freezeTimer <= sf::Time::Zero)
        {
            m_state.m_isPlayerFrozen = false;
            EntityUtils::forEachAlive(m_state.m_entities, [](Entity& entity) {
                if (auto* fish = dynamic_cast<Fish*>(&entity))
                    fish->setFrozen(false);
            });
        }
    }

    if (m_state.m_hasControlsReversed)
    {
        m_state.m_controlReverseTimer -= deltaTime;
        if (m_state.m_controlReverseTimer <= sf::Time::Zero)
        {
            m_state.m_hasControlsReversed = false;
            m_state.m_player->setControlsReversed(false);
        }
    }

    if (m_state.m_isPlayerStunned)
    {
        m_state.m_stunTimer -= deltaTime;
        if (m_state.m_stunTimer <= sf::Time::Zero)
            m_state.m_isPlayerStunned = false;
    }
}

void PlayLogic::applyEnvironmentalForces(sf::Time deltaTime)
{
    if (!m_state.m_isPlayerStunned)
    {
        sf::Vector2f current = m_state.m_environmentSystem->getOceanCurrentForce(m_state.m_player->getPosition());
        m_state.m_player->setVelocity(m_state.m_player->getVelocity() + current * deltaTime.asSeconds() * 0.3f);
    }

    EntityUtils::forEachAlive(m_state.m_entities, [this, deltaTime](Entity& entity) {
        sf::Vector2f cur = m_state.m_environmentSystem->getOceanCurrentForce(entity.getPosition());
        entity.setVelocity(entity.getVelocity() + cur * deltaTime.asSeconds() * 0.1f);
    });
}

void PlayLogic::handlePowerUpCollision(PowerUp& powerUp)
{
    switch (powerUp.getPowerUpType())
    {
    case PowerUpType::ScoreDoubler:
        m_state.m_powerUpManager->activatePowerUp(powerUp.getPowerUpType(), powerUp.getDuration());
        createParticleEffect(powerUp.getPosition(), Constants::SCORE_DOUBLER_COLOR);
        break;
    case PowerUpType::FrenzyStarter:
        m_state.m_frenzySystem->forceFrenzy();
        createParticleEffect(powerUp.getPosition(), Constants::FRENZY_STARTER_COLOR);
        break;
    case PowerUpType::SpeedBoost:
        m_state.m_powerUpManager->activatePowerUp(powerUp.getPowerUpType(), powerUp.getDuration());
        m_state.m_player->applySpeedBoost(m_state.m_powerUpManager->getSpeedMultiplier(), powerUp.getDuration());
        m_state.getGame().getSoundPlayer().play(SoundEffectID::SpeedStart);
        createParticleEffect(powerUp.getPosition(), Constants::SPEED_BOOST_COLOR);
        break;
    case PowerUpType::Freeze:
        m_state.m_powerUpManager->activatePowerUp(powerUp.getPowerUpType(), powerUp.getDuration());
        applyFreeze();
        createParticleEffect(powerUp.getPosition(), sf::Color::Cyan, 20);
        break;
    case PowerUpType::ExtraLife:
        m_state.m_gameState.playerLives++;
        m_state.getGame().getSoundPlayer().play(SoundEffectID::LifePowerup);
        createParticleEffect(powerUp.getPosition(), sf::Color::Green, 15);
        break;
    case PowerUpType::AddTime:
        break;
    }
}

void PlayLogic::handleOysterCollision(PermanentOyster* oyster)
{
    if (oyster->canDamagePlayer() && !m_state.m_player->isInvulnerable())
    {
        m_state.m_player->takeDamage();
        handlePlayerDeath();
        createParticleEffect(m_state.m_player->getPosition(), Constants::DAMAGE_PARTICLE_COLOR);
    }
    else if (oyster->canBeEaten())
    {
        oyster->onCollect();
        m_state.getGame().getSoundPlayer().play(SoundEffectID::OysterPearl);

        int points = oyster->hasBlackPearl() ? Constants::BLACK_OYSTER_POINTS : Constants::WHITE_OYSTER_POINTS;

        m_state.m_player->addPoints(points);
        m_state.m_player->grow(oyster->getGrowthPoints());

        int frenzyMultiplier = m_state.m_frenzySystem->getMultiplier();
        float powerUpMultiplier = m_state.m_powerUpManager->getScoreMultiplier();

        m_state.m_scoreSystem->addScore(ScoreEventType::BonusCollected, oyster->getPoints(),
                                        oyster->getPosition(), frenzyMultiplier, powerUpMultiplier);

        createParticleEffect(oyster->getPosition(),
                            oyster->hasBlackPearl() ? Constants::BLACK_PEARL_COLOR : Constants::WHITE_PEARL_COLOR);
    }
}

void PlayLogic::applyFreeze()
{
    m_state.m_isPlayerFrozen = true;
    m_state.m_freezeTimer = sf::seconds(5.0f);
    m_state.getGame().getSoundPlayer().play(SoundEffectID::FreezePowerup);

    EntityUtils::forEachAlive(m_state.m_entities, [](Entity& entity) {
        if (auto* fish = dynamic_cast<Fish*>(&entity))
            fish->setFrozen(true);
    });
}

void PlayLogic::reverseControls()
{
    m_state.m_hasControlsReversed = true;
    m_state.m_player->setControlsReversed(true);
}

void PlayLogic::checkWinCondition()
{
    if (m_state.m_player->getPoints() >= Constants::POINTS_TO_WIN)
        triggerWinSequence();
}

void PlayLogic::triggerWinSequence()
{
    m_state.getGame().getMusicPlayer().play(MusicID::StageCleared, false);
    m_state.m_gameState.gameWon = true;
    m_state.m_gameState.enemiesFleeing = true;
    m_state.m_gameState.winTimer = sf::Time::Zero;

    makeAllEnemiesFlee();
    showMessage("LEVEL COMPLETE!\n\nEat the fleeing fish for bonus points!");

    m_state.m_fishSpawner->setLevel(-1);
    m_state.m_bonusItemManager->setStarfishEnabled(false);
    m_state.m_bonusItemManager->setPowerUpsEnabled(false);
}

void PlayLogic::makeAllEnemiesFlee()
{
    EntityUtils::forEachAlive(m_state.m_entities, [](Entity& e) {
        if (auto* fish = dynamic_cast<Fish*>(&e))
            fish->startFleeing();
    });
}

bool PlayLogic::areAllEnemiesGone() const
{
    return std::none_of(m_state.m_entities.begin(), m_state.m_entities.end(),
        [](const auto& ent) { return ent->isAlive() && dynamic_cast<const Fish*>(ent.get()) != nullptr; });
}

void PlayLogic::handlePlayerDeath()
{
    if (m_state.m_player->isInvulnerable())
        return;

    m_state.m_camera.freeze(m_state.m_player->getPosition());

    m_state.m_gameState.playerLives--;
    m_state.getGame().getMusicPlayer().play(MusicID::PlayerDies, false);
    m_state.m_musicResumePending = m_state.m_gameState.playerLives > 0;
    if (m_state.m_musicResumePending)
        m_state.m_musicResumeTimer = sf::seconds(2.0f);
    m_state.m_player->die();

    if (m_state.m_gameState.playerLives <= 0)
    {
        gameOver();
    }
    else
    {
        m_state.m_respawnPending = true;
        m_state.m_respawnTimer = Constants::RESPAWN_DELAY;
    }
}

void PlayLogic::advanceLevel()
{
    int levelScore = m_state.m_scoreSystem->getCurrentScore();
    const auto& fishCounts = m_state.m_scoreSystem->getFishCounts();
    bool triggerBonus = (m_state.m_gameState.currentLevel % 3 == 0);
    StageSummaryState::configure(m_state.m_gameState.currentLevel + 1,
                                 levelScore, fishCounts, triggerBonus);
    m_state.m_levelCounts.clear();

    m_state.m_gameState.currentLevel++;
    m_state.m_gameState.totalScore += levelScore;

    m_state.updateBackground(m_state.m_gameState.currentLevel);

    if (m_state.m_gameState.currentLevel % 3 == 0)
    {
        EnvironmentType newEnv = static_cast<EnvironmentType>((static_cast<int>(m_state.m_environmentSystem->getCurrentEnvironment()) + 1) % 3);
        m_state.m_environmentSystem->setEnvironment(newEnv);
    }

    m_state.m_environmentSystem->setRandomTimeOfDay();

    resetLevel();
    updateLevelDifficulty();

    m_state.m_hudSystem->clearMessage();
    m_state.m_bonusStageTriggered = false;

    StageIntroState::configure(m_state.m_gameState.currentLevel, false);
    m_state.deferAction([this]() { m_state.requestStackPush(StateID::StageSummary); });
}

void PlayLogic::resetLevel()
{
    m_state.m_player->fullReset();

    m_state.m_player->setPosition(m_state.m_camera.getWorldSize() * 0.5f);
    m_state.m_camera.getView().setCenter(m_state.m_player->getPosition());

    m_state.m_gameState.levelComplete = false;
    m_state.m_gameState.gameWon = false;
    m_state.m_gameState.enemiesFleeing = false;
    m_state.m_gameState.levelTime = sf::Time::Zero;

    m_state.m_entities.clear();
    m_state.m_bonusItems.clear();
    m_state.m_hazards.clear();
    m_state.m_particleSystem->clear();

    m_state.m_scoreSystem->reset();
    m_state.m_frenzySystem->reset();
    m_state.m_powerUpManager->reset();
    m_state.m_growthMeter->reset();
    m_state.m_oysterManager->resetAll();

    m_state.m_isPlayerFrozen = false;
    m_state.m_hasControlsReversed = false;
    m_state.m_isPlayerStunned = false;
    m_state.m_controlReverseTimer = sf::Time::Zero;
    m_state.m_freezeTimer = sf::Time::Zero;
    m_state.m_stunTimer = sf::Time::Zero;

    m_state.m_bonusItemManager->setStarfishEnabled(true);
    m_state.m_bonusItemManager->setPowerUpsEnabled(true);
}

void PlayLogic::gameOver()
{
    GameStats& stats = GameStats::getInstance();
    stats.finalScore = m_state.m_gameState.totalScore;
    stats.levelReached = m_state.m_gameState.currentLevel;
    stats.survivalTime = m_state.m_gameState.levelTime.asSeconds();
    stats.newHighScore = stats.finalScore > stats.highScore;
    if (stats.newHighScore)
        stats.highScore = stats.finalScore;

    m_state.requestStackClear();
    m_state.requestStackPush(StateID::GameOver);
}

void PlayLogic::updateLevelDifficulty()
{
    m_state.m_bonusItemManager->setLevel(m_state.m_gameState.currentLevel);

    SpecialFishConfig config;
    float levelMultiplier = 1.0f + (m_state.m_gameState.currentLevel - 1) * Constants::DIFFICULTY_INCREMENT;

    config.barracudaSpawnRate = Constants::BARRACUDA_SPAWN_RATE * levelMultiplier;
    config.pufferfishSpawnRate = Constants::PUFFERFISH_SPAWN_RATE * levelMultiplier;
    config.angelfishSpawnRate = Constants::ANGELFISH_SPAWN_RATE * levelMultiplier;
    config.poisonFishSpawnRate = Constants::POISONFISH_SPAWN_RATE * levelMultiplier;
    config.schoolSpawnChance = std::min(Constants::MAX_SCHOOL_SPAWN_CHANCE,
        Constants::SCHOOL_SPAWN_CHANCE * levelMultiplier);

    m_state.m_fishSpawner->setSpecialFishConfig(config);
    m_state.m_fishSpawner->setLevel(m_state.m_gameState.currentLevel);
}

void PlayLogic::checkBonusStage()
{
    if (!m_state.m_bonusStageTriggered && m_state.m_gameState.levelComplete && !m_state.m_returningFromBonusStage)
    {
        if (m_state.m_gameState.currentLevel % 3 == 0)
        {
            m_state.m_bonusStageTriggered = true;
            m_state.m_savedLevel = m_state.m_gameState.currentLevel;

            BonusStageType bonusType = static_cast<BonusStageType>(
                std::uniform_int_distribution<int>(0, 2)(m_state.m_randomEngine));

            m_state.deferAction([this, bonusType]() {
                m_state.m_returningFromBonusStage = true;
                auto& cfg = BonusStageConfig::getInstance();
                cfg.type = bonusType;
                cfg.playerLevel = m_state.m_savedLevel;
                StageIntroState::configure(0, true, StateID::BonusStage);
                m_state.requestStackPush(StateID::StageIntro);
            });
        }
    }
}

void PlayLogic::createParticleEffect(const sf::Vector2f& position, const sf::Color& color, int count)
{
    m_state.m_particleSystem->createEffect(position, color, count);
}

void PlayLogic::updateHUD()
{
    auto activePowerUps = m_state.m_powerUpManager->getActivePowerUps();
    m_state.m_hudSystem->update(
        m_state.m_scoreSystem->getCurrentScore(),
        m_state.m_gameState.playerLives,
        m_state.m_gameState.currentLevel,
        m_state.m_scoreSystem->getChainBonus(),
        activePowerUps,
        m_state.m_isPlayerFrozen, m_state.m_freezeTimer,
        m_state.m_hasControlsReversed, m_state.m_controlReverseTimer,
        m_state.m_isPlayerStunned, m_state.m_stunTimer,
        m_state.m_metrics.currentFPS);
}

void PlayLogic::updatePerformanceMetrics(sf::Time deltaTime)
{
    m_state.m_metrics.frameCount++;
    m_state.m_metrics.fpsUpdateTime += deltaTime;

    if (m_state.m_metrics.fpsUpdateTime >= Constants::FPS_UPDATE_INTERVAL)
    {
        m_state.m_metrics.currentFPS = static_cast<float>(m_state.m_metrics.frameCount) /
            m_state.m_metrics.fpsUpdateTime.asSeconds();
        m_state.m_metrics.frameCount = 0;
        m_state.m_metrics.fpsUpdateTime = sf::Time::Zero;
    }
}

void PlayLogic::updateCamera()
{
    if (!m_state.m_player)
        return;

    m_state.m_camera.update(m_state.m_player->getPosition());
}

void PlayLogic::showMessage(const std::string& message)
{
    m_state.m_hudSystem->showMessage(message);
}

} // namespace FishGame
