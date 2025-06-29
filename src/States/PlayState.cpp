#include "PlayState.h"
#include "Game.h"
#include "CollisionDetector.h"
#include "Fish.h"
#include "ExtendedPowerUps.h"
#include "GameOverState.h"
#include "StageIntroState.h"
#include "StageSummaryState.h"
#include "MusicPlayer.h"
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
        , m_hazardSpawnTimer(sf::Time::Zero)
        , m_extendedPowerUpSpawnTimer(sf::Time::Zero)
        , m_bonusStageTriggered(false)
        , m_returningFromBonusStage(false)
        , m_savedLevel(1)
        , m_metrics()
        , m_particleSystem(std::make_unique<ParticleSystem>())
        , m_randomEngine(std::random_device{}())
        , m_angleDist(0.0f, 360.0f)
        , m_speedDist(Constants::MIN_PARTICLE_SPEED, Constants::MAX_PARTICLE_SPEED)
        , m_positionDist(Constants::SAFE_SPAWN_PADDING,
            Constants::WINDOW_WIDTH - Constants::SAFE_SPAWN_PADDING)
        , m_hazardTypeDist(0, 1)
        , m_powerUpTypeDist(0, 2)
        , m_initialized(false)
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

        m_worldSize = windowSize;

        m_view = window.getDefaultView();
        m_view.zoom(0.8f);
        m_view.setCenter(m_worldSize * 0.5f);
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

    }


    void PlayState::handleEvent(const sf::Event& event)
    {
        if (m_isPlayerStunned || getGame().getCurrentState<StageIntroState>())
            return;

        m_inputHandler.setReversed(m_hasControlsReversed);
        m_inputHandler.processEvent(event, [this](const sf::Event& processedEvent)
        {
        switch (processedEvent.type)
        {
        case sf::Event::KeyPressed:
            switch (processedEvent.key.code)
            {
            case sf::Keyboard::Escape:
                deferAction([this]() {
                    requestStackPop();
                    requestStackPush(StateID::Menu);
                    });
                break;

            /*case sf::Keyboard::Enter:
                if (m_gameState.levelComplete)
                {
                    advanceLevel();
                }
                break;*/

            case sf::Keyboard::P:
                deferAction([this]() {
                    StageIntroState::configure(m_gameState.currentLevel, false);
                    requestStackPush(StateID::StageIntro);
                    });
                break;

                // Movement keys will be handled in Player::handleInput()
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
            // Mouse input disabled
            break;

        case sf::Event::MouseButtonPressed:
            break;

        default:
            break;
        }
    });
}

    bool PlayState::update(sf::Time deltaTime)
    {
        updatePerformanceMetrics(deltaTime);
        updateGameplay(deltaTime);
        processDeferredActions();
        return false;
    }

    void PlayState::updateGameplay(sf::Time deltaTime)
    {
        m_gameState.levelTime += deltaTime;

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
                m_cameraFrozen = false;
                createParticleEffect(m_player->getPosition(), Constants::RESPAWN_PARTICLE_COLOR);
            }
        }

        // Update environment system
        m_environmentSystem->update(deltaTime);

        // Update all systems
        updateSystems(deltaTime);

        // Update player effects
        updateEffectTimers(deltaTime);

        // Apply environmental effects
        applyEnvironmentalForces(deltaTime);

        // Check bonus stage
        checkBonusStage();

        // Handle game state
        if (m_gameState.gameWon)
        {
            m_gameState.winTimer += deltaTime;

            bool timerExpired = m_gameState.winTimer >=
                Constants::WIN_SEQUENCE_DURATION;
            bool noEnemies = m_gameState.enemiesFleeing && areAllEnemiesGone();

            if (timerExpired || noEnemies)
            {
                m_gameState.enemiesFleeing = false;
                m_gameState.levelComplete = true;
                advanceLevel();
            }

            if (m_gameState.levelComplete)
            {
                return;
            }
        }
        else if (!m_gameState.levelComplete)
        {
            checkWinCondition();
        }

        // Update all entities
        updateAllEntities(deltaTime);

        // Handle spawning
        if (!m_gameState.gameWon)
        {
            m_fishSpawner->update(deltaTime, m_gameState.currentLevel);
            auto& spawnedFish = m_fishSpawner->getSpawnedFish();
            std::move(spawnedFish.begin(), spawnedFish.end(), std::back_inserter(m_entities));
            spawnedFish.clear();

            if (shouldSpawnSpecialEntity(m_hazardSpawnTimer, m_hazardSpawnInterval))
            {
                spawnRandomHazard();
            }

            if (shouldSpawnSpecialEntity(m_extendedPowerUpSpawnTimer, m_extendedPowerUpInterval))
            {
                spawnRandomPowerUp();
            }
        }

        // Update bonus items
        m_bonusItemManager->update(deltaTime);
        auto newItems = m_bonusItemManager->collectSpawnedItems();
        std::move(newItems.begin(), newItems.end(), std::back_inserter(m_bonusItems));

        // Update particles
        m_particleSystem->update(deltaTime);

        // Remove dead entities from all containers
        EntityUtils::removeDeadEntities(m_entities);
        EntityUtils::removeDeadEntities(m_hazards);

        // Special handling for bonus items (check expiration too)
        m_bonusItems.erase(
            std::remove_if(m_bonusItems.begin(), m_bonusItems.end(),
                [](const auto& item) {
                    return !item || !item->isAlive() || item->hasExpired();
                }),
            m_bonusItems.end()
        );


        // Check collisions
        checkCollisions();

        // Update HUD
        updateHUD();

        // Update camera to follow player
        updateCamera();
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
        if (!m_isPlayerStunned)
        {
            m_player->update(deltaTime);
        }
    }

    void PlayState::updateAllEntities(sf::Time deltaTime)
    {
        StateUtils::updateEntities(m_entities, deltaTime);
        StateUtils::updateEntities(m_bonusItems, deltaTime);
        StateUtils::updateEntities(m_hazards, deltaTime);

        // Apply specific AI updates
        StateUtils::applyToEntities(m_entities, [this, deltaTime](Entity& entity) {
            if (auto* fish = dynamic_cast<Fish*>(&entity))
            {
                if (!fish->isStunned())
                {
                    fish->updateAI(m_entities, m_player.get(), deltaTime);
                }
            }
            });
    }

    void PlayState::updateEffectTimers(sf::Time deltaTime)
    {
        // Update freeze timer
        if (m_isPlayerFrozen)
        {
            m_freezeTimer -= deltaTime;
            if (m_freezeTimer <= sf::Time::Zero)
            {
                m_isPlayerFrozen = false;
                StateUtils::applyToEntities(m_entities, [](Entity& entity) {
                    if (auto* fish = dynamic_cast<Fish*>(&entity))
                    {
                        fish->setFrozen(false);
                    }
                    });
            }
        }

        // Update control reversal timer
        if (m_hasControlsReversed)
        {
            m_controlReverseTimer -= deltaTime;
            if (m_controlReverseTimer <= sf::Time::Zero)
            {
                m_hasControlsReversed = false;
                m_player->setControlsReversed(false);
            }
        }

        // Update stun timer
        if (m_isPlayerStunned)
        {
            m_stunTimer -= deltaTime;
            if (m_stunTimer <= sf::Time::Zero)
            {
                m_isPlayerStunned = false;
            }
        }
    }

    void PlayState::applyEnvironmentalForces(sf::Time deltaTime)
    {
        // Apply ocean currents to player
        if (!m_isPlayerStunned)
        {
            sf::Vector2f playerCurrent = m_environmentSystem->getOceanCurrentForce(m_player->getPosition());
            m_player->setVelocity(m_player->getVelocity() + playerCurrent * deltaTime.asSeconds() * 0.3f);
        }

        // Apply currents to all entities
        StateUtils::applyToEntities(m_entities, [this, deltaTime](Entity& entity) {
            sf::Vector2f current = m_environmentSystem->getOceanCurrentForce(entity.getPosition());
            entity.setVelocity(entity.getVelocity() + current * deltaTime.asSeconds() * 0.1f);
            });
    }

    bool PlayState::shouldSpawnSpecialEntity(sf::Time& timer, float interval)
    {
        timer += sf::seconds(1.0f / 60.0f);
        if (timer.asSeconds() >= interval)
        {
            timer = sf::Time::Zero;
            return true;
        }
        return false;
    }

    void PlayState::spawnRandomHazard()
    {
        std::unique_ptr<Hazard> hazard;

        switch (m_hazardTypeDist(m_randomEngine))
        {
        case 0:
            if (m_gameState.currentLevel >= 6)
            {
                hazard = std::make_unique<Bomb>();
                static_cast<Bomb*>(hazard.get())->initializeSprite(getGame().getSpriteManager());
            }
            break;
        case 1:
            if (m_gameState.currentLevel >= 4)
            {
                hazard = std::make_unique<Jellyfish>();
                static_cast<Jellyfish*>(hazard.get())->initializeSprite(getGame().getSpriteManager());
                hazard->setVelocity(0.0f, 20.0f);
            }
            break;
        }

        if (hazard)
        {
            hazard->setPosition(generateRandomPosition());
            m_hazards.push_back(std::move(hazard));
        }
    }

    void PlayState::spawnRandomPowerUp()
    {
        std::unique_ptr<PowerUp> powerUp;

        int type = m_powerUpTypeDist(m_randomEngine);
        if (m_gameState.currentLevel < 2 && (type == 0 || type == 2))
            type = 1; // Freeze and SpeedBoost unlocked from level 2

        switch (type)
        {
        case 0:
            powerUp = std::make_unique<FreezePowerUp>();
            if (auto* freeze = dynamic_cast<FreezePowerUp*>(powerUp.get()))
            {
                freeze->setFont(getGame().getFonts().get(Fonts::Main));
            }
            break;
        case 1:
            powerUp = std::make_unique<ExtraLifePowerUp>();
            if (auto* life = dynamic_cast<ExtraLifePowerUp*>(powerUp.get()))
            {
                life->initializeSprite(getGame().getSpriteManager());
            }
            break;
        case 2:
            powerUp = std::make_unique<SpeedBoostPowerUp>();
            if (auto* speed = dynamic_cast<SpeedBoostPowerUp*>(powerUp.get()))
            {
                speed->initializeSprite(getGame().getSpriteManager());
            }
            break;
        }

        if (powerUp)
        {
            sf::Vector2f pos = generateRandomPosition();
            powerUp->setPosition(pos);
            powerUp->m_baseY = pos.y;
            m_bonusItems.push_back(std::move(powerUp));
        }
    }

    sf::Vector2f PlayState::generateRandomPosition()
    {
        return sf::Vector2f(m_positionDist(m_randomEngine),
            std::uniform_real_distribution<float>(Constants::SAFE_SPAWN_PADDING,
                Constants::WINDOW_HEIGHT - Constants::SAFE_SPAWN_PADDING)(
                    m_randomEngine));
    }

    void PlayState::checkCollisions()
    {
        // Player vs various containers
        EntityUtils::forEachAlive(m_entities, [this](Entity& entity) {
            if (EntityUtils::areColliding(*m_player, entity)) {
                FishCollisionHandler{ this }(entity);
            }
            });

        EntityUtils::forEachAlive(m_bonusItems, [this](Entity& item) {
            if (EntityUtils::areColliding(*m_player, *static_cast<BonusItem*>(&item))) {
                BonusItemCollisionHandler{ this }(*static_cast<BonusItem*>(&item));
            }
            });

        EntityUtils::forEachAlive(m_hazards, [this](Entity& hazard) {
            if (EntityUtils::areColliding(*m_player, *static_cast<Hazard*>(&hazard))) {
                HazardCollisionHandler{ this }(*static_cast<Hazard*>(&hazard));
            }
            });

        // Player vs oysters
        if (m_gameState.currentLevel >= 2)
        {
            m_oysterManager->checkCollisions(*m_player,
                [this](PermanentOyster* oyster) { handleOysterCollision(oyster); });
        }

        // Fish vs fish collisions
        StateUtils::processCollisionsBetween(m_entities, m_entities,
            [this](Entity& entity1, Entity& entity2) {
                auto* fish1 = dynamic_cast<Fish*>(&entity1);
                auto* fish2 = dynamic_cast<Fish*>(&entity2);

                if (!fish1 || !fish2) return;

                if (fish1->canEat(*fish2))
                {
                    if (auto* poison = dynamic_cast<PoisonFish*>(fish2))
                    {
                        fish1->setPoisoned(poison->getPoisonDuration());
                        createParticleEffect(fish1->getPosition(), sf::Color::Magenta, 10);
                    }
                    fish1->playEatAnimation();
                    fish2->destroy();
                    createParticleEffect(fish2->getPosition(), Constants::DEATH_PARTICLE_COLOR);
                }
                else if (fish2->canEat(*fish1))
                {
                    if (auto* poison = dynamic_cast<PoisonFish*>(fish1))
                    {
                        fish2->setPoisoned(poison->getPoisonDuration());
                        createParticleEffect(fish2->getPosition(), sf::Color::Magenta, 10);
                    }
                    fish2->playEatAnimation();
                    fish1->destroy();
                    createParticleEffect(fish1->getPosition(), Constants::DEATH_PARTICLE_COLOR);
                }
            });

        // Fish vs hazards - using the global FishCollisionHandler from FishCollisionHandler.h
        ::FishGame::FishCollisionHandler::processFishHazardCollisions(
            m_entities, m_hazards, &getGame().getSoundPlayer());

        // Bomb explosions
        processBombExplosions(m_entities, m_hazards);

        // Check tail-bite opportunities
        StateUtils::applyToEntities(m_entities, [this](Entity& entity) {
            if (m_player->attemptTailBite(entity))
            {
                createParticleEffect(m_player->getPosition(), Constants::TAILBITE_PARTICLE_COLOR);
            }
            });

        // Enemy fish vs oysters
        if (m_gameState.currentLevel >= 2)
        {
            StateUtils::applyToEntities(m_entities, [this](Entity& entity) {
                if (auto* fish = dynamic_cast<Fish*>(&entity))
                {
                    m_oysterManager->checkCollisions(*fish,
                        [this, fish](PermanentOyster* oyster) {
                            if (oyster->canDamagePlayer())
                            {
                                fish->destroy();
                                createParticleEffect(fish->getPosition(), Constants::DEATH_PARTICLE_COLOR);
                                createParticleEffect(oyster->getPosition(), Constants::OYSTER_IMPACT_COLOR);
                            }
                        });
                }
                });
        }
    }

    void PlayState::FishCollisionHandler::operator()(Entity& fish) const
    {
        if (state->m_player->isInvulnerable() || state->m_isPlayerStunned)
            return;

        if (auto* puffer = dynamic_cast<Pufferfish*>(&fish))
        {
            if (puffer->isInflated())
            {
                if (!state->m_player->hasRecentlyTakenDamage())
                {
                    puffer->pushEntity(*state->m_player);
                    state->getGame().getSoundPlayer().play(SoundEffectID::PufferBounce);
                    int penalty = Constants::PUFFERFISH_SCORE_PENALTY;
                    state->m_scoreSystem->setCurrentScore(
                        std::max(0, state->m_scoreSystem->getCurrentScore() - penalty)
                    );
                    state->createParticleEffect(state->m_player->getPosition(),
                        Constants::PUFFERFISH_IMPACT_COLOR);
                }
            }
            else if (state->m_player->canEat(fish))
            {
                if (state->m_player->attemptEat(fish))
                {
                    state->m_levelCounts[puffer->getTextureID()]++;
                    SoundEffectID effect = SoundEffectID::Bite2;
                    state->getGame().getSoundPlayer().play(effect);
                    fish.destroy();
                    state->createParticleEffect(fish.getPosition(), Constants::EAT_PARTICLE_COLOR);
                }
            }
            else if (puffer->canEat(*state->m_player) && !state->m_player->hasRecentlyTakenDamage())
            {
                state->m_player->takeDamage();
                state->createParticleEffect(state->m_player->getPosition(), Constants::DAMAGE_PARTICLE_COLOR);
                state->handlePlayerDeath();
            }
        }
        else if (auto* angelfish = dynamic_cast<Angelfish*>(&fish))
        {
            if (state->m_player->canEat(fish) && state->m_player->attemptEat(fish))
            {
                state->m_levelCounts[angelfish->getTextureID()]++;
                state->getGame().getSoundPlayer().play(SoundEffectID::Bite1);
                state->createParticleEffect(fish.getPosition(),
                    Constants::ANGELFISH_PARTICLE_COLOR, Constants::ANGELFISH_PARTICLE_COUNT);
                fish.destroy();
            }
        }
        else if (auto* poison = dynamic_cast<PoisonFish*>(&fish))
        {
            if (state->m_player->canEat(fish) && state->m_player->attemptEat(fish))
            {
                state->reverseControls();
                state->m_controlReverseTimer = poison->getPoisonDuration();
                state->m_player->applyPoisonEffect(poison->getPoisonDuration());
                state->getGame().getSoundPlayer().play(SoundEffectID::PlayerPoison);
                state->createParticleEffect(fish.getPosition(), sf::Color::Magenta, 15);
                state->createParticleEffect(state->m_player->getPosition(), sf::Color::Magenta, 10);
                state->m_levelCounts[poison->getTextureID()]++;
                fish.destroy();
            }
        }
        else if (auto* regularFish = dynamic_cast<Fish*>(&fish))
        {
            bool playerCanEat = state->m_player->canEat(fish);
            bool fishCanEatPlayer = regularFish->canEat(*state->m_player);

            if (playerCanEat && state->m_player->attemptEat(fish))
            {
                state->m_levelCounts[regularFish->getTextureID()]++;
                SoundEffectID effect = SoundEffectID::Bite1;
                switch (regularFish->getSize())
                {
                case FishSize::Small:
                    effect = SoundEffectID::Bite1;
                    break;
                case FishSize::Medium:
                    effect = SoundEffectID::Bite2;
                    break;
                case FishSize::Large:
                    effect = SoundEffectID::Bite3;
                    break;
                }
                state->getGame().getSoundPlayer().play(effect);
                fish.destroy();
                state->createParticleEffect(fish.getPosition(), Constants::EAT_PARTICLE_COLOR);
            }
            else if (fishCanEatPlayer && !state->m_player->hasRecentlyTakenDamage())
            {
                regularFish->playEatAnimation();
                state->m_player->takeDamage();
                state->createParticleEffect(state->m_player->getPosition(), Constants::DAMAGE_PARTICLE_COLOR);
                state->handlePlayerDeath();
            }
        }
    }

    void PlayState::BonusItemCollisionHandler::operator()(BonusItem& item) const
    {
        item.onCollect();

        if (auto* powerUp = dynamic_cast<PowerUp*>(&item))
        {
            state->handlePowerUpCollision(*powerUp);
        }
        else
        {
            if (item.getBonusType() == BonusType::Starfish)
            {
                state->m_levelCounts[TextureID::Starfish]++;
                state->m_scoreSystem->recordFish(TextureID::Starfish);
                state->getGame().getSoundPlayer().play(SoundEffectID::StarPickup);
            }
            int frenzyMultiplier = state->m_frenzySystem->getMultiplier();
            float powerUpMultiplier = state->m_powerUpManager->getScoreMultiplier();

            state->m_scoreSystem->addScore(ScoreEventType::BonusCollected, item.getPoints(),
                item.getPosition(), frenzyMultiplier, powerUpMultiplier);

            state->createParticleEffect(item.getPosition(), Constants::BONUS_PARTICLE_COLOR);
        }
    }

    void PlayState::HazardCollisionHandler::operator()(Hazard& hazard) const
    {
        if (state->m_player->isInvulnerable())
            return;

        switch (hazard.getHazardType())
        {
        case HazardType::Bomb:
            if (auto* bomb = dynamic_cast<Bomb*>(&hazard))
            {
                bomb->onContact(*state->m_player);
                state->getGame().getSoundPlayer().play(SoundEffectID::MineExplode);
                state->m_player->takeDamage();
                state->handlePlayerDeath();
                state->createParticleEffect(state->m_player->getPosition(), sf::Color::Red, 20);
            }
            break;

        case HazardType::Jellyfish:
            if (auto* jelly = dynamic_cast<Jellyfish*>(&hazard))
            {
                jelly->onContact(*state->m_player);
                state->m_isPlayerStunned = true;
                state->m_stunTimer = jelly->getStunDuration();
                state->m_player->setVelocity(0.0f, 0.0f);
                state->getGame().getSoundPlayer().play(SoundEffectID::PlayerStunned);
                state->createParticleEffect(state->m_player->getPosition(), sf::Color(255, 255, 0, 150), 10);
            }
            break;
        }
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
            applyFreeze();
            // sound handled in applyFreeze
            createParticleEffect(powerUp.getPosition(), sf::Color::Cyan, 20);
            break;

        case PowerUpType::ExtraLife:
            m_gameState.playerLives++;
            getGame().getSoundPlayer().play(SoundEffectID::LifePowerup);
            createParticleEffect(powerUp.getPosition(), sf::Color::Green, 15);
            break;

        case PowerUpType::AddTime:
            // Currently handled only in bonus stages
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

    void PlayState::applyFreeze()
    {
        m_isPlayerFrozen = true;
        m_freezeTimer = sf::seconds(5.0f);
        getGame().getSoundPlayer().play(SoundEffectID::FreezePowerup);

        StateUtils::applyToEntities(m_entities, [](Entity& entity) {
            if (auto* fish = dynamic_cast<Fish*>(&entity))
            {
                fish->setFrozen(true);
            }
            });
    }

    void PlayState::reverseControls()
    {
        m_hasControlsReversed = true;
        m_player->setControlsReversed(true);
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
        StateUtils::applyToEntities(m_entities, [](Entity& entity) {
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
        m_cameraFrozen = true;
        m_cameraFreezePos = m_player->getPosition();
        sf::Vector2f halfSize = m_view.getSize() * 0.5f;
        m_cameraFreezePos.x = std::clamp(m_cameraFreezePos.x, halfSize.x, m_worldSize.x - halfSize.x);
        m_cameraFreezePos.y = std::clamp(m_cameraFreezePos.y, halfSize.y, m_worldSize.y - halfSize.y);

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

        m_hudSystem->clearMessage();
        m_bonusStageTriggered = false;

        StageIntroState::configure(m_gameState.currentLevel, false);
        deferAction([this]() { requestStackPush(StateID::StageSummary); });
    }

    void PlayState::resetLevel()
    {
        m_player->fullReset();

        // Start player in the middle of the world
        m_player->setPosition(m_worldSize * 0.5f);
        m_view.setCenter(m_player->getPosition());

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

        m_isPlayerFrozen = false;
        m_hasControlsReversed = false;
        m_isPlayerStunned = false;
        m_controlReverseTimer = sf::Time::Zero;
        m_freezeTimer = sf::Time::Zero;
        m_stunTimer = sf::Time::Zero;

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

    void PlayState::updateHUD()
    {
        auto activePowerUps = m_powerUpManager->getActivePowerUps();
        m_hudSystem->update(
            m_scoreSystem->getCurrentScore(),
            m_gameState.playerLives,
            m_gameState.currentLevel,
            m_scoreSystem->getChainBonus(),
            activePowerUps,
            m_isPlayerFrozen, m_freezeTimer,
            m_hasControlsReversed, m_controlReverseTimer,
            m_isPlayerStunned, m_stunTimer,
            m_metrics.currentFPS);
    }

    void PlayState::updatePerformanceMetrics(sf::Time deltaTime)
    {
        m_metrics.frameCount++;
        m_metrics.fpsUpdateTime += deltaTime;

        if (m_metrics.fpsUpdateTime >= Constants::FPS_UPDATE_INTERVAL)
        {
            m_metrics.currentFPS = static_cast<float>(m_metrics.frameCount) /
                m_metrics.fpsUpdateTime.asSeconds();
            m_metrics.frameCount = 0;
            m_metrics.fpsUpdateTime = sf::Time::Zero;

            // FPS value stored for HUD system
        }
    }

    void PlayState::updateCamera()
    {
        if (!m_player)
            return;

        if (m_cameraFrozen)
        {
            m_view.setCenter(m_cameraFreezePos);
            return;
        }

        sf::Vector2f target = m_player->getPosition();
        sf::Vector2f halfSize = m_view.getSize() * 0.5f;

        // Clamp horizontally
        if (m_worldSize.x > m_view.getSize().x)
        {
            target.x = std::clamp(target.x, halfSize.x, m_worldSize.x - halfSize.x);
        }
        else
        {
            target.x = m_worldSize.x * 0.5f;
        }

        // Clamp vertically
        if (m_worldSize.y > m_view.getSize().y)
        {
            target.y = std::clamp(target.y, halfSize.y, m_worldSize.y - halfSize.y);
        }
        else
        {
            target.y = m_worldSize.y * 0.5f;
        }

        sf::Vector2f current = m_view.getCenter();
        sf::Vector2f newCenter = current + (target - current) * m_cameraSmoothing;
        m_view.setCenter(newCenter);
    }

    void PlayState::showMessage(const std::string& message)
    {
        m_hudSystem->showMessage(message);
    }

    void PlayState::render()
    {
        auto& window = getGame().getWindow();
        auto defaultView = window.getView();
        window.setView(m_view);

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

            resetLevel();
            updateLevelDifficulty();
            updateBackground(m_gameState.currentLevel);

            // Mouse control disabled

            m_hudSystem->clearMessage();
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
