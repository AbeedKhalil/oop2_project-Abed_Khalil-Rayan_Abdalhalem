﻿#include "PlayState.h"
#include "Game.h"
#include "CollisionDetector.h"
#include "Fish.h"
#include "ExtendedPowerUps.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <numeric>

namespace FishGame
{
    PlayState::PlayState(Game& game)
        : State(game)
        , m_player(std::make_unique<Player>())
        , m_fishSpawner(std::make_unique<EnhancedFishSpawner>(getGame().getWindow().getSize()))
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
        , m_levelStats()
        , m_hud()
        , m_isPlayerFrozen(false)
        , m_hasControlsReversed(false)
        , m_isPlayerStunned(false)
        , m_controlReverseTimer(sf::Time::Zero)
        , m_freezeTimer(sf::Time::Zero)
        , m_stunTimer(sf::Time::Zero)
        , m_hazardSpawnTimer(sf::Time::Zero)
        , m_extendedPowerUpSpawnTimer(sf::Time::Zero)
        , m_levelsUntilBonus(3)
        , m_bonusStageTriggered(false)
        , m_returningFromBonusStage(false)
        , m_savedLevel(1)
        , m_metrics()
        , m_particles()
        , m_randomEngine(std::random_device{}())
        , m_angleDist(0.0f, 360.0f)
        , m_speedDist(Constants::MIN_PARTICLE_SPEED, Constants::MAX_PARTICLE_SPEED)
        , m_positionDist(100.0f, 1820.0f)
        , m_hazardTypeDist(0, 1)
        , m_powerUpTypeDist(0, 2)
    {
        initializeSystems();

        // Reserve capacity for containers
        m_entities.reserve(Constants::MAX_ENTITIES);
        m_bonusItems.reserve(Constants::MAX_BONUS_ITEMS);
        m_hazards.reserve(20);
        m_particles.reserve(Constants::MAX_PARTICLES);
    }

    template<typename SystemType>
    SystemType* PlayState::createAndStoreSystem(const std::string& name, const sf::Font& font)
    {
        auto system = std::make_unique<SystemType>(font);
        SystemType* ptr = system.get();
        m_systems[name] = std::unique_ptr<void, std::function<void(void*)>>(
            system.release(),
            [](void* p) { delete static_cast<SystemType*>(p); }
        );
        return ptr;
    }

    void PlayState::initializeSystems()
    {
        auto& window = getGame().getWindow();
        auto& font = getGame().getFonts().get(Fonts::Main);

        // Create game systems
        m_growthMeter = createAndStoreSystem<GrowthMeter>("growth", font);
        m_frenzySystem = createAndStoreSystem<FrenzySystem>("frenzy", font);
        m_scoreSystem = createAndStoreSystem<ScoreSystem>("score", font);

        // Special initialization for other systems
        auto powerUpManager = std::make_unique<PowerUpManager>();
        m_powerUpManager = powerUpManager.get();
        m_systems["powerup"] = std::unique_ptr<void, std::function<void(void*)>>(
            powerUpManager.release(),
            [](void* p) { delete static_cast<PowerUpManager*>(p); }
        );

        auto bonusManager = std::make_unique<BonusItemManager>(window.getSize(), font);
        m_bonusItemManager = bonusManager.get();
        m_systems["bonus"] = std::unique_ptr<void, std::function<void(void*)>>(
            bonusManager.release(),
            [](void* p) { delete static_cast<BonusItemManager*>(p); }
        );

        auto oysterManager = std::make_unique<FixedOysterManager>(window.getSize());
        m_oysterManager = oysterManager.get();
        m_systems["oyster"] = std::unique_ptr<void, std::function<void(void*)>>(
            oysterManager.release(),
            [](void* p) { delete static_cast<FixedOysterManager*>(p); }
        );

        // Initialize environment system
        m_environmentSystem->setEnvironment(EnvironmentType::OpenOcean);
        m_environmentSystem->pauseDayNightCycle();

        // Initialize player with systems
        m_player->setWindowBounds(window.getSize());
        m_player->initializeSystems(m_growthMeter, m_frenzySystem, m_powerUpManager, m_scoreSystem);

        // Configure spawners
        m_fishSpawner->setSchoolingSystem(m_schoolingSystem.get());

        SpecialFishConfig specialConfig{
            Constants::BARRACUDA_SPAWN_RATE,
            Constants::PUFFERFISH_SPAWN_RATE,
            Constants::ANGELFISH_SPAWN_RATE,
            Constants::SCHOOL_SPAWN_CHANCE
        };
        m_fishSpawner->setSpecialFishConfig(specialConfig);

        // Position UI elements
        m_growthMeter->setPosition(Constants::GROWTH_METER_X, window.getSize().y - Constants::GROWTH_METER_Y_OFFSET);
        m_frenzySystem->setPosition(window.getSize().x / 2.0f, Constants::FRENZY_Y_POSITION);

        // Initialize HUD
        initializeHUD();

        // Configure initial state
        m_fishSpawner->setLevel(m_gameState.currentLevel);
        m_bonusItemManager->setOysterEnabled(false);

        updateHUD();
    }

    void PlayState::initializeHUD()
    {
        auto& font = getGame().getFonts().get(Fonts::Main);
        auto& window = getGame().getWindow();

        // Lambda for initializing text objects
        auto initText = [&font](sf::Text& text, unsigned int size, const sf::Vector2f& position,
            const sf::Color& color = Constants::HUD_TEXT_COLOR) {
                text.setFont(font);
                text.setCharacterSize(size);
                text.setFillColor(color);
                text.setPosition(position);
            };

        // Initialize HUD texts
        initText(m_hud.scoreText, Constants::HUD_FONT_SIZE,
            sf::Vector2f(Constants::HUD_MARGIN, Constants::HUD_MARGIN));
        initText(m_hud.livesText, Constants::HUD_FONT_SIZE,
            sf::Vector2f(Constants::HUD_MARGIN, Constants::HUD_MARGIN + Constants::HUD_LINE_SPACING));
        initText(m_hud.levelText, Constants::HUD_FONT_SIZE,
            sf::Vector2f(Constants::HUD_MARGIN, Constants::HUD_MARGIN + Constants::HUD_LINE_SPACING * 2));
        initText(m_hud.chainText, Constants::HUD_SMALL_FONT_SIZE,
            sf::Vector2f(Constants::HUD_MARGIN, Constants::HUD_MARGIN + Constants::HUD_LINE_SPACING * 3));
        initText(m_hud.powerUpText, Constants::HUD_SMALL_FONT_SIZE,
            sf::Vector2f(window.getSize().x - Constants::POWERUP_TEXT_X_OFFSET, Constants::HUD_MARGIN + Constants::HUD_LINE_SPACING));
        initText(m_hud.fpsText, Constants::HUD_FONT_SIZE,
            sf::Vector2f(window.getSize().x - Constants::FPS_TEXT_X_OFFSET, Constants::HUD_MARGIN));
        initText(m_hud.environmentText, 20,
            sf::Vector2f(window.getSize().x - 300.0f, 100.0f));
        initText(m_hud.effectsText, 18,
            sf::Vector2f(50.0f, window.getSize().y - 100.0f), sf::Color::Yellow);

        // Special handling for message text
        m_hud.messageText.setFont(font);
        m_hud.messageText.setCharacterSize(Constants::MESSAGE_FONT_SIZE);
        m_hud.messageText.setFillColor(Constants::MESSAGE_COLOR);
        m_hud.messageText.setOutlineColor(Constants::MESSAGE_OUTLINE_COLOR);
        m_hud.messageText.setOutlineThickness(Constants::MESSAGE_OUTLINE_THICKNESS);
    }

    void PlayState::handleEvent(const sf::Event& event)
    {
        if (m_isPlayerStunned)
            return;

        // Handle controls reversal using STL transform
        sf::Event processedEvent = event;
        if (m_hasControlsReversed && event.type == sf::Event::KeyPressed)
        {
            static const std::unordered_map<sf::Keyboard::Key, sf::Keyboard::Key> reverseMap = {
                {sf::Keyboard::W, sf::Keyboard::S}, {sf::Keyboard::S, sf::Keyboard::W},
                {sf::Keyboard::A, sf::Keyboard::D}, {sf::Keyboard::D, sf::Keyboard::A},
                {sf::Keyboard::Up, sf::Keyboard::Down}, {sf::Keyboard::Down, sf::Keyboard::Up},
                {sf::Keyboard::Left, sf::Keyboard::Right}, {sf::Keyboard::Right, sf::Keyboard::Left}
            };

            if (auto it = reverseMap.find(event.key.code); it != reverseMap.end())
            {
                processedEvent.key.code = it->second;
            }
        }

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

            case sf::Keyboard::Enter:
                if (m_gameState.levelComplete)
                {
                    advanceLevel();
                }
                break;

            case sf::Keyboard::P:
                deferAction([this]() {
                    requestStackPush(StateID::Pause);
                    });
                break;

            default:
                break;
            }
            break;

        case sf::Event::MouseMoved:
        {
            auto windowSize = getGame().getWindow().getSize();
            float mouseX = static_cast<float>(event.mouseMove.x);
            float mouseY = static_cast<float>(event.mouseMove.y);

            if (m_hasControlsReversed)
            {
                mouseX = windowSize.x - mouseX;
                mouseY = windowSize.y - mouseY;
            }

            m_player->followMouse(sf::Vector2f(mouseX, mouseY));
        }
        break;

        default:
            break;
        }
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

            if (m_gameState.enemiesFleeing && areAllEnemiesGone())
            {
                m_gameState.enemiesFleeing = false;
                m_gameState.levelComplete = true;
                showMessage("Level Complete!\nPress ENTER to continue");
                return;
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

        // Update particles with parallel execution
        std::for_each(m_particles.begin(), m_particles.end(),
            [deltaTime](ParticleEffect& particle) {
                particle.lifetime -= deltaTime;
                particle.shape.move(particle.velocity * deltaTime.asSeconds());
                particle.alpha = std::max(0.0f, particle.alpha - Constants::PARTICLE_FADE_RATE * deltaTime.asSeconds());
                sf::Color color = particle.shape.getFillColor();
                color.a = static_cast<sf::Uint8>(particle.alpha);
                particle.shape.setFillColor(color);
            });

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

        // Remove expired particles
        m_particles.erase(
            std::remove_if(m_particles.begin(), m_particles.end(),
                [](const auto& particle) {
                    return particle.lifetime <= sf::Time::Zero;
                }),
            m_particles.end()
        );

        // Check collisions
        checkCollisions();

        // Update HUD
        updateHUD();
    }

    void PlayState::updateSystems(sf::Time deltaTime)
    {
        // Update core systems
        m_frenzySystem->update(deltaTime);
        m_powerUpManager->update(deltaTime);
        m_scoreSystem->update(deltaTime);
        m_growthMeter->update(deltaTime);
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
        // Update all entities with freeze effect
        auto freezeModifier = m_isPlayerFrozen ? 0.1f : 1.0f;

        updateEntities(m_entities, deltaTime);
        updateEntities(m_bonusItems, deltaTime);
        updateEntities(m_hazards, deltaTime);

        // Apply specific AI updates and effects
        applyEffectToEntities(m_entities, [this, deltaTime, freezeModifier](Entity& entity) {
            if (auto* fish = dynamic_cast<Fish*>(&entity))
            {
                if (m_isPlayerFrozen)
                {
                    fish->setVelocity(fish->getVelocity() * freezeModifier);
                }

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
                applyEffectToEntities(m_entities, [](Entity& entity) {
                    if (auto* fish = dynamic_cast<Fish*>(&entity))
                    {
                        fish->setVelocity(fish->getVelocity() * 10.0f);
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
        applyEffectToEntities(m_entities, [this, deltaTime](Entity& entity) {
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
            hazard = std::make_unique<Bomb>();
            break;
        case 1:
            hazard = std::make_unique<Jellyfish>();
            hazard->setVelocity(0.0f, 20.0f);
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

        switch (m_powerUpTypeDist(m_randomEngine))
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
            break;
        case 2:
            powerUp = std::make_unique<SpeedBoostPowerUp>();
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
            std::uniform_real_distribution<float>(100.0f, 980.0f)(m_randomEngine));
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
        m_oysterManager->checkCollisions(*m_player,
            [this](PermanentOyster* oyster) { handleOysterCollision(oyster); });

        // Fish vs fish collisions
        processCollisionsBetween(m_entities, m_entities,
            [this](Entity& entity1, Entity& entity2) {
                auto* fish1 = dynamic_cast<Fish*>(&entity1);
                auto* fish2 = dynamic_cast<Fish*>(&entity2);

                if (!fish1 || !fish2) return;

                if (fish1->canEat(*fish2))
                {
                    fish2->destroy();
                    createParticleEffect(fish2->getPosition(), Constants::DEATH_PARTICLE_COLOR);
                }
                else if (fish2->canEat(*fish1))
                {
                    fish1->destroy();
                    createParticleEffect(fish1->getPosition(), Constants::DEATH_PARTICLE_COLOR);
                }
            });

        // Fish vs hazards - using the global FishCollisionHandler from FishCollisionHandler.h
        ::FishGame::FishCollisionHandler::processFishHazardCollisions(m_entities, m_hazards);

        // Bomb explosions
        processBombExplosions(m_entities, m_hazards);

        // Check tail-bite opportunities
        applyEffectToEntities(m_entities, [this](Entity& entity) {
            if (m_player->attemptTailBite(entity))
            {
                createParticleEffect(m_player->getPosition(), Constants::TAILBITE_PARTICLE_COLOR);
            }
            });

        // Enemy fish vs oysters
        applyEffectToEntities(m_entities, [this](Entity& entity) {
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

    void PlayState::FishCollisionHandler::operator()(Entity& fish) const
    {
        if (state->m_player->isInvulnerable())
            return;

        if (auto* puffer = dynamic_cast<Pufferfish*>(&fish))
        {
            if (puffer->isInflated())
            {
                if (!state->m_player->hasRecentlyTakenDamage())
                {
                    puffer->pushEntity(*state->m_player);
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
        else if (auto* angel = dynamic_cast<Angelfish*>(&fish))
        {
            if (state->m_player->canEat(fish) && state->m_player->attemptEat(fish))
            {
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
                state->createParticleEffect(fish.getPosition(), sf::Color::Magenta, 15);
                state->createParticleEffect(state->m_player->getPosition(), sf::Color::Magenta, 10);
                fish.destroy();
            }
        }
        else if (auto* regularFish = dynamic_cast<Fish*>(&fish))
        {
            bool playerCanEat = state->m_player->canEat(fish);
            bool fishCanEatPlayer = regularFish->canEat(*state->m_player);

            if (playerCanEat && state->m_player->attemptEat(fish))
            {
                fish.destroy();
                state->createParticleEffect(fish.getPosition(), Constants::EAT_PARTICLE_COLOR);
            }
            else if (fishCanEatPlayer && !state->m_player->hasRecentlyTakenDamage())
            {
                state->m_player->takeDamage();
                state->createParticleEffect(state->m_player->getPosition(), Constants::DAMAGE_PARTICLE_COLOR);
                state->handlePlayerDeath();
            }
        }
    }

    void PlayState::BonusItemCollisionHandler::operator()(BonusItem& item) const
    {
        if (auto* oyster = dynamic_cast<PearlOyster*>(&item))
        {
            if (!oyster->isOpen())
                return;
        }

        item.onCollect();

        if (auto* powerUp = dynamic_cast<PowerUp*>(&item))
        {
            state->handlePowerUpCollision(*powerUp);
        }
        else
        {
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
            createParticleEffect(powerUp.getPosition(), Constants::SPEED_BOOST_COLOR);
            break;

        case PowerUpType::Invincibility:
            m_player->applyInvincibility(powerUp.getDuration());
            createParticleEffect(powerUp.getPosition(), Constants::INVINCIBILITY_COLOR);
            break;

        case PowerUpType::Freeze:
            m_powerUpManager->activatePowerUp(powerUp.getPowerUpType(), powerUp.getDuration());
            applyFreeze();
            createParticleEffect(powerUp.getPosition(), sf::Color::Cyan, 20);
            break;

        case PowerUpType::ExtraLife:
            m_gameState.playerLives++;
            createParticleEffect(powerUp.getPosition(), sf::Color::Green, 15);
            break;

        case PowerUpType::Shield:
            m_powerUpManager->activatePowerUp(powerUp.getPowerUpType(), powerUp.getDuration());
            m_player->applyInvincibility(powerUp.getDuration());
            createParticleEffect(powerUp.getPosition(), sf::Color(255, 215, 0), 20);
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

        applyEffectToEntities(m_entities, [](Entity& entity) {
            if (auto* fish = dynamic_cast<Fish*>(&entity))
            {
                fish->setVelocity(fish->getVelocity() * 0.1f);
            }
            });
    }

    void PlayState::reverseControls()
    {
        m_hasControlsReversed = true;
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
        applyEffectToEntities(m_entities, [](Entity& entity) {
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

        m_gameState.playerLives--;
        m_player->die();

        if (m_gameState.playerLives <= 0)
        {
            gameOver();
        }
        else
        {
            m_player->respawn();
            createParticleEffect(m_player->getPosition(), Constants::RESPAWN_PARTICLE_COLOR);
        }
    }

    void PlayState::advanceLevel()
    {
        m_gameState.currentLevel++;
        m_gameState.totalScore += m_scoreSystem->getCurrentScore();
        m_scoreSystem->addToTotalScore(m_scoreSystem->getCurrentScore());

        if (m_gameState.currentLevel % 3 == 0)
        {
            EnvironmentType newEnv = static_cast<EnvironmentType>(
                (static_cast<int>(m_environmentSystem->getCurrentEnvironment()) + 1) % 3);
            m_environmentSystem->setEnvironment(newEnv);
        }

        m_environmentSystem->setRandomTimeOfDay();

        resetLevel();
        updateLevelDifficulty();

        m_hud.messageText.setString("");
        m_bonusStageTriggered = false;
    }

    void PlayState::resetLevel()
    {
        m_player->fullReset();

        m_gameState.levelComplete = false;
        m_gameState.gameWon = false;
        m_gameState.enemiesFleeing = false;
        m_gameState.levelTime = sf::Time::Zero;

        m_entities.clear();
        m_bonusItems.clear();
        m_hazards.clear();
        m_particles.clear();

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
        m_gameState.playerLives = Constants::INITIAL_LIVES;
        resetLevel();
        showMessage("Game Over! Press ESC for menu");
    }

    void PlayState::updateLevelDifficulty()
    {
        m_bonusItemManager->setLevel(m_gameState.currentLevel);

        SpecialFishConfig config;
        float levelMultiplier = 1.0f + (m_gameState.currentLevel - 1) * Constants::DIFFICULTY_INCREMENT;

        config.barracudaSpawnRate = Constants::BARRACUDA_SPAWN_RATE * levelMultiplier;
        config.pufferfishSpawnRate = Constants::PUFFERFISH_SPAWN_RATE * levelMultiplier;
        config.angelfishSpawnRate = Constants::ANGELFISH_SPAWN_RATE * levelMultiplier;
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
                    requestStackPush(StateID::BonusStage);
                    });
            }
        }
    }

    void PlayState::createParticleEffect(const sf::Vector2f& position, const sf::Color& color, int count)
    {
        m_particles.reserve(m_particles.size() + count);

        std::generate_n(std::back_inserter(m_particles), count,
            [this, &position, &color]() {
                ParticleEffect particle;
                particle.shape = sf::CircleShape(Constants::PARTICLE_RADIUS);
                particle.shape.setFillColor(color);
                particle.shape.setPosition(position);

                float angle = m_angleDist(m_randomEngine) * Constants::DEG_TO_RAD;
                float speed = m_speedDist(m_randomEngine);
                particle.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

                particle.lifetime = sf::seconds(Constants::PARTICLE_LIFETIME);
                particle.alpha = Constants::PARTICLE_INITIAL_ALPHA;

                return particle;
            });
    }

    void PlayState::updateHUD()
    {
        auto formatText = [](sf::Text& text, const auto&... args) {
            std::ostringstream stream;
            ((stream << args), ...);
            text.setString(stream.str());
            };

        formatText(m_hud.scoreText,
            "Score: ", m_scoreSystem->getCurrentScore(),
            " | Points: ", m_player->getPoints(), "/", Constants::POINTS_TO_WIN);

        formatText(m_hud.livesText, "Lives: ", m_gameState.playerLives);

        int levelsUntilBonus = 3 - (m_gameState.currentLevel % 3);
        if (levelsUntilBonus == 3) levelsUntilBonus = 0;

        formatText(m_hud.levelText,
            "Level: ", m_gameState.currentLevel,
            " | Stage: ", m_player->getCurrentStage(), "/", Constants::MAX_STAGES,
            m_gameState.gameWon ? " | COMPLETE!" : "",
            levelsUntilBonus > 0 ? " | Bonus in: " : " | Bonus after this level!",
            levelsUntilBonus > 0 ? std::to_string(levelsUntilBonus) + " levels" : "");

        if (m_scoreSystem->getChainBonus() > 0)
        {
            formatText(m_hud.chainText, "Chain Bonus: +", m_scoreSystem->getChainBonus());
        }
        else
        {
            m_hud.chainText.setString("");
        }

        auto activePowerUps = m_powerUpManager->getActivePowerUps();
        if (!activePowerUps.empty())
        {
            std::ostringstream powerUpStream;
            powerUpStream << "Active Power-Ups:\n";

            std::for_each(activePowerUps.begin(), activePowerUps.end(),
                [this, &powerUpStream](PowerUpType type) {
                    // Create local map to avoid static const capture issue
                    std::unordered_map<PowerUpType, std::string> names = {
                        {PowerUpType::ScoreDoubler, "2X Score"},
                        {PowerUpType::SpeedBoost, "Speed Boost"},
                        {PowerUpType::Invincibility, "Invincible"},
                        {PowerUpType::Freeze, "Freeze"},
                        {PowerUpType::Shield, "Shield"}
                    };

                    if (auto it = names.find(type); it != names.end())
                    {
                        powerUpStream << it->second;
                        sf::Time remaining = m_powerUpManager->getRemainingTime(type);
                        if (remaining > sf::Time::Zero)
                        {
                            powerUpStream << " - " << std::fixed << std::setprecision(1)
                                << remaining.asSeconds() << "s";
                        }
                        powerUpStream << "\n";
                    }
                });

            m_hud.powerUpText.setString(powerUpStream.str());
        }
        else
        {
            m_hud.powerUpText.setString("");
        }

        std::ostringstream envStream;
        envStream << "Environment: ";
        switch (m_environmentSystem->getCurrentEnvironment())
        {
        case EnvironmentType::CoralReef:
            envStream << "Coral Reef";
            break;
        case EnvironmentType::OpenOcean:
            envStream << "Open Ocean";
            break;
        case EnvironmentType::KelpForest:
            envStream << "Kelp Forest";
            break;
        }

        envStream << "\nTime: ";
        switch (m_environmentSystem->getCurrentTimeOfDay())
        {
        case TimeOfDay::Day:
            envStream << "Day";
            break;
        case TimeOfDay::Dusk:
            envStream << "Dusk";
            break;
        case TimeOfDay::Night:
            envStream << "Night";
            break;
        case TimeOfDay::Dawn:
            envStream << "Dawn";
            break;
        }

        m_hud.environmentText.setString(envStream.str());

        std::ostringstream effectStream;
        if (m_isPlayerFrozen)
            effectStream << "FREEZE ACTIVE: " << std::fixed << std::setprecision(1)
            << m_freezeTimer.asSeconds() << "s\n";
        if (m_hasControlsReversed)
            effectStream << "CONTROLS REVERSED: " << std::fixed << std::setprecision(1)
            << m_controlReverseTimer.asSeconds() << "s\n";
        if (m_isPlayerStunned)
            effectStream << "STUNNED: " << std::fixed << std::setprecision(1)
            << m_stunTimer.asSeconds() << "s\n";
        m_hud.effectsText.setString(effectStream.str());
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

            std::ostringstream fpsStream;
            fpsStream << std::fixed << std::setprecision(1) << "FPS: " << m_metrics.currentFPS;
            m_hud.fpsText.setString(fpsStream.str());
        }
    }

    void PlayState::showMessage(const std::string& message)
    {
        m_hud.messageText.setString(message);
        centerText(m_hud.messageText);
    }

    void PlayState::centerText(sf::Text& text)
    {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        auto windowSize = getGame().getWindow().getSize();
        text.setPosition(windowSize.x / 2.0f, windowSize.y / 2.0f);
    }

    void PlayState::render()
    {
        auto& window = getGame().getWindow();

        window.draw(*m_environmentSystem);

        renderContainer(m_hazards, window);
        renderContainer(m_entities, window);

        m_oysterManager->draw(window);

        renderContainer(m_bonusItems, window);

        window.draw(*m_player);

        std::for_each(m_particles.begin(), m_particles.end(),
            [&window](const ParticleEffect& particle) {
                window.draw(particle.shape);
            });

        m_scoreSystem->drawFloatingScores(window);
        window.draw(*m_growthMeter);
        window.draw(*m_frenzySystem);

        // Render HUD texts
        window.draw(m_hud.scoreText);
        window.draw(m_hud.livesText);
        window.draw(m_hud.levelText);
        window.draw(m_hud.chainText);
        window.draw(m_hud.powerUpText);
        window.draw(m_hud.fpsText);
        window.draw(m_hud.environmentText);
        window.draw(m_hud.effectsText);

        if (m_gameState.gameWon || m_gameState.levelComplete)
        {
            sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
            overlay.setFillColor(Constants::OVERLAY_COLOR);
            window.draw(overlay);
            window.draw(m_hud.messageText);
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

            m_hud.messageText.setString("");
        }
        else
        {
            resetLevel();
            m_gameState.currentLevel = 1;
            m_gameState.playerLives = Constants::INITIAL_LIVES;
            m_gameState.totalScore = 0;
            m_levelsUntilBonus = 3;
            m_bonusStageTriggered = false;
            m_returningFromBonusStage = false;
            m_savedLevel = 1;
        }
    }
}