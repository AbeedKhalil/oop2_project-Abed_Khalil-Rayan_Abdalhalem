#include "PlayState.h"
#include "Game.h"
#include "CollisionDetector.h"
#include "Fish.h"
#include "ExtendedPowerUps.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <numeric>
#include <execution>

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
        , m_returningFromBonusStage(false)  // Initialize new flag
        , m_savedLevel(1)                    // Initialize saved level
        , m_metrics()
        , m_particles()
        , m_randomEngine(std::random_device{}())
        , m_angleDist(0.0f, 360.0f)
        , m_speedDist(Constants::MIN_PARTICLE_SPEED, Constants::MAX_PARTICLE_SPEED)
        , m_hazardTypeDist(0, 2)
        , m_extendedPowerUpDist(0, 2)
    {
        initializeSystems();

        // Reserve capacity for containers
        m_entities.reserve(Constants::MAX_ENTITIES);
        m_bonusItems.reserve(Constants::MAX_BONUS_ITEMS);
        m_hazards.reserve(20);
        m_particles.reserve(Constants::MAX_PARTICLES);
    }

    void PlayState::initializeSystems()
    {
        auto& window = getGame().getWindow();
        auto& font = getGame().getFonts().get(Fonts::Main);

        // Create game systems using factory pattern
        auto createSystem = [this, &font]<typename T>(const std::string & name) -> T* {
            auto system = std::make_unique<T>(font);
            T* ptr = system.get();
            m_systems[name] = std::unique_ptr<void, std::function<void(void*)>>(
                system.release(),
                [](void* p) { delete static_cast<T*>(p); }
            );
            return ptr;
        };

        // Initialize systems
        m_growthMeter = createSystem.template operator() < GrowthMeter > ("growth");
        m_frenzySystem = createSystem.template operator() < FrenzySystem > ("frenzy");
        m_scoreSystem = createSystem.template operator() < ScoreSystem > ("score");

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
        m_environmentSystem->pauseDayNightCycle();  // Pause day/night cycle
        m_environmentSystem->setOnEnvironmentChange([this](EnvironmentType type) {
            // Adjust spawn rates based on environment
            switch (type)
            {
            case EnvironmentType::CoralReef:
                m_bonusItemManager->setStarfishEnabled(true);
                m_bonusItemManager->setPowerUpsEnabled(true);
                break;
            case EnvironmentType::KelpForest:
                // More hazards in kelp forest
                break;
            case EnvironmentType::OpenOcean:
                // Default rates
                break;
            }
            });

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
        auto initText = [&font](sf::Text& text, unsigned int size, const sf::Vector2f& position) {
            text.setFont(font);
            text.setCharacterSize(size);
            text.setFillColor(Constants::HUD_TEXT_COLOR);
            text.setPosition(position);
            };

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

        // Environment and effects text
        initText(m_hud.environmentText, 20,
            sf::Vector2f(window.getSize().x - 300.0f, 100.0f));
        initText(m_hud.effectsText, 18,
            sf::Vector2f(50.0f, window.getSize().y - 100.0f));
        m_hud.effectsText.setFillColor(sf::Color::Yellow);

        // Message text setup
        m_hud.messageText.setFont(font);
        m_hud.messageText.setCharacterSize(Constants::MESSAGE_FONT_SIZE);
        m_hud.messageText.setFillColor(Constants::MESSAGE_COLOR);
        m_hud.messageText.setOutlineColor(Constants::MESSAGE_OUTLINE_COLOR);
        m_hud.messageText.setOutlineThickness(Constants::MESSAGE_OUTLINE_THICKNESS);

        // Configure initial state
        m_fishSpawner->setLevel(m_gameState.currentLevel);
        m_bonusItemManager->setOysterEnabled(false); // Using fixed oysters

        updateHUD();
    }

    void PlayState::handleEvent(const sf::Event& event)
    {
        // Handle stunned state
        if (m_isPlayerStunned)
            return;

        // Handle controls reversal
        sf::Event processedEvent = event;
        if (m_hasControlsReversed && event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
            case sf::Keyboard::W:
                processedEvent.key.code = sf::Keyboard::S;
                break;
            case sf::Keyboard::S:
                processedEvent.key.code = sf::Keyboard::W;
                break;
            case sf::Keyboard::A:
                processedEvent.key.code = sf::Keyboard::D;
                break;
            case sf::Keyboard::D:
                processedEvent.key.code = sf::Keyboard::A;
                break;
            case sf::Keyboard::Up:
                processedEvent.key.code = sf::Keyboard::Down;
                break;
            case sf::Keyboard::Down:
                processedEvent.key.code = sf::Keyboard::Up;
                break;
            case sf::Keyboard::Left:
                processedEvent.key.code = sf::Keyboard::Right;
                break;
            case sf::Keyboard::Right:
                processedEvent.key.code = sf::Keyboard::Left;
                break;
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
                // Fixed condition - check if level is complete regardless of enemies fleeing
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
            if (!m_hasControlsReversed)
            {
                m_player->followMouse(sf::Vector2f(
                    static_cast<float>(event.mouseMove.x),
                    static_cast<float>(event.mouseMove.y)
                ));
            }
            else
            {
                // Reverse mouse movement
                auto windowSize = getGame().getWindow().getSize();
                m_player->followMouse(sf::Vector2f(
                    windowSize.x - static_cast<float>(event.mouseMove.x),
                    windowSize.y - static_cast<float>(event.mouseMove.y)
                ));
            }
            break;

        default:
            break;
        }
    }

    bool PlayState::update(sf::Time deltaTime)
    {
        // Update performance metrics
        updatePerformanceMetrics(deltaTime);

        // Update gameplay
        updateGameplay(deltaTime);

        // Process deferred actions
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
        updatePlayerEffects(deltaTime);

        // Apply environmental effects
        updateEnvironmentalEffects(deltaTime);

        // Check bonus stage
        checkBonusStage();

        // Handle game state
        if (m_gameState.gameWon)
        {
            m_gameState.winTimer += deltaTime;

            // Check if all enemies are gone
            if (m_gameState.enemiesFleeing && areAllEnemiesGone())
            {
                m_gameState.enemiesFleeing = false;
                m_gameState.levelComplete = true;
                showMessage("Level Complete!\nPress ENTER to continue");
                return; // Stop processing after showing message
            }

            // Don't process further updates if level is complete
            if (m_gameState.levelComplete)
            {
                return;
            }
        }
        else if (!m_gameState.levelComplete)
        {
            // Only check win condition if not already won or complete
            checkWinCondition();
        }

        // Update entities
        updateEntities(deltaTime);

        // Handle spawning (unless game is won)
        if (!m_gameState.gameWon)
        {
            m_fishSpawner->update(deltaTime, m_gameState.currentLevel);
            auto& spawnedFish = m_fishSpawner->getSpawnedFish();
            processSpawnedEntities(spawnedFish);

            // Spawn hazards
            spawnHazards(deltaTime);

            // Spawn extended power-ups
            spawnExtendedPowerUps();
        }

        // Update bonus items
        m_bonusItemManager->update(deltaTime);
        auto newItems = m_bonusItemManager->collectSpawnedItems();
        std::move(newItems.begin(), newItems.end(), std::back_inserter(m_bonusItems));

        // Update containers
        updateContainer(m_bonusItems, deltaTime);
        updateContainer(m_entities, deltaTime);
        updateContainer(m_hazards, deltaTime);

        // Update particles
        std::for_each(std::execution::par_unseq, m_particles.begin(), m_particles.end(),
            [deltaTime](ParticleEffect& particle) {
                particle.lifetime -= deltaTime;
                particle.shape.move(particle.velocity * deltaTime.asSeconds());
                particle.alpha = std::max(0.0f, particle.alpha - Constants::PARTICLE_FADE_RATE * deltaTime.asSeconds());
                sf::Color color = particle.shape.getFillColor();
                color.a = static_cast<sf::Uint8>(particle.alpha);
                particle.shape.setFillColor(color);
            });

        // Remove dead entities
        removeDeadEntities(m_entities, [](const auto& entity) { return !entity->isAlive(); });
        removeDeadEntities(m_bonusItems, [](const auto& item) {
            return !item->isAlive() || item->hasExpired();
            });
        removeDeadEntities(m_hazards, [](const auto& hazard) { return !hazard->isAlive(); });
        removeDeadEntities(m_particles, [](const auto& particle) {
            return particle.lifetime <= sf::Time::Zero;
            });

        // Check collisions
        checkCollisions();
        checkHazardCollisions();

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
            processSpawnedEntities(schoolFish);

            // Try to re-add small fish to schools
            std::vector<std::unique_ptr<Entity>> remainingEntities;

            std::partition_copy(
                std::make_move_iterator(m_entities.begin()),
                std::make_move_iterator(m_entities.end()),
                std::back_inserter(remainingEntities),
                std::back_inserter(remainingEntities),
                [this](const std::unique_ptr<Entity>& entity) {
                    if (auto* smallFish = dynamic_cast<SmallFish*>(entity.get()))
                    {
                        if (smallFish->isAlive())
                        {
                            auto schoolMember = std::make_unique<SmallFish>(m_gameState.currentLevel);
                            schoolMember->setPosition(smallFish->getPosition());
                            schoolMember->setVelocity(smallFish->getVelocity());
                            schoolMember->setWindowBounds(getGame().getWindow().getSize());
                            return m_schoolingSystem->tryAddToSchool(std::move(schoolMember));
                        }
                    }
                    return false;
                });

            m_entities = std::move(remainingEntities);
        }

        // Update player
        if (!m_isPlayerStunned)
        {
            m_player->update(deltaTime);
        }
    }

    void PlayState::updateEntities(sf::Time deltaTime)
    {
        // Update all entities and their AI
        std::for_each(std::execution::par_unseq, m_entities.begin(), m_entities.end(),
            [this, deltaTime](const auto& entity) {
                entity->update(deltaTime);

                if (Fish* fish = dynamic_cast<Fish*>(entity.get()))
                {
                    // Apply freeze effect
                    if (m_isPlayerFrozen)
                    {
                        fish->setVelocity(fish->getVelocity() * 0.1f);
                    }

                    fish->updateAI(m_entities, m_player.get(), deltaTime);
                }
            });

        // Handle special fish behaviors
        std::for_each(m_entities.begin(), m_entities.end(),
            [this, deltaTime](const auto& entity) {
                if (auto* pufferfish = dynamic_cast<Pufferfish*>(entity.get()))
                {
                    if (pufferfish->isInflated() && pufferfish->canPushEntity(*m_player))
                    {
                        // This is handled in collision detection
                    }
                }
                else if (auto* angelfish = dynamic_cast<Angelfish*>(entity.get()))
                {
                    angelfish->updateAI(m_entities, m_player.get(), deltaTime);
                }
            });
    }

    void PlayState::updatePlayerEffects(sf::Time deltaTime)
    {
        // Handle freeze effect
        if (m_isPlayerFrozen)
        {
            m_freezeTimer -= deltaTime;
            if (m_freezeTimer <= sf::Time::Zero)
            {
                m_isPlayerFrozen = false;
                // Unfreeze all entities
                std::for_each(m_entities.begin(), m_entities.end(),
                    [](auto& entity) {
                        if (Fish* fish = dynamic_cast<Fish*>(entity.get()))
                        {
                            fish->setVelocity(fish->getVelocity() * 10.0f); // Restore speed
                        }
                    });
            }
        }

        // Handle control reversal
        if (m_hasControlsReversed)
        {
            m_controlReverseTimer -= deltaTime;
            if (m_controlReverseTimer <= sf::Time::Zero)
            {
                m_hasControlsReversed = false;
            }
        }

        // Handle stun effect
        if (m_isPlayerStunned)
        {
            m_stunTimer -= deltaTime;
            if (m_stunTimer <= sf::Time::Zero)
            {
                m_isPlayerStunned = false;
            }
        }
    }

    void PlayState::updateEnvironmentalEffects(sf::Time deltaTime)
    {
        // Apply ocean currents to all entities
        sf::Vector2f playerCurrent = m_environmentSystem->getOceanCurrentForce(m_player->getPosition());

        if (!m_isPlayerStunned)
        {
            m_player->setVelocity(m_player->getVelocity() + playerCurrent * deltaTime.asSeconds() * 0.3f);
        }

        // Apply currents to fish with reduced effect
        std::for_each(m_entities.begin(), m_entities.end(),
            [this, deltaTime](auto& entity) {
                sf::Vector2f current = m_environmentSystem->getOceanCurrentForce(entity->getPosition());
                entity->setVelocity(entity->getVelocity() + current * deltaTime.asSeconds() * 0.1f);
            });

        // Apply aggression multiplier
        float aggressionMult = m_environmentSystem->getPredatorAggressionMultiplier();
        // This would affect AI behavior in fish update
    }

    void PlayState::spawnHazards(sf::Time deltaTime)
    {
        m_hazardSpawnTimer += deltaTime;

        if (m_hazardSpawnTimer.asSeconds() >= m_hazardSpawnInterval)
        {
            m_hazardSpawnTimer = sf::Time::Zero;

            int hazardType = m_hazardTypeDist(m_randomEngine);
            std::unique_ptr<Hazard> hazard;

            switch (hazardType)
            {
            case 0: // Bomb
                hazard = std::make_unique<Bomb>();
                break;
            case 1: // Jellyfish
                hazard = std::make_unique<Jellyfish>();
                break;
            }

            if (hazard)
            {
                // Random position
                float x = std::uniform_real_distribution<float>(100.0f, 1820.0f)(m_randomEngine);
                float y = std::uniform_real_distribution<float>(100.0f, 980.0f)(m_randomEngine);
                hazard->setPosition(x, y);

                // Set velocity for moving hazards
                if (dynamic_cast<PoisonFish*>(hazard.get()))
                {
                    bool fromLeft = m_randomEngine() % 2 == 0;
                    hazard->setVelocity(fromLeft ? 100.0f : -100.0f, 0.0f);
                }
                else if (dynamic_cast<Jellyfish*>(hazard.get()))
                {
                    hazard->setVelocity(0.0f, 20.0f);
                }

                m_hazards.push_back(std::move(hazard));
            }
        }
    }

    void PlayState::spawnExtendedPowerUps()
    {
        m_extendedPowerUpSpawnTimer += sf::seconds(1.0f / 60.0f); // Frame time

        if (m_extendedPowerUpSpawnTimer.asSeconds() >= m_extendedPowerUpInterval)
        {
            m_extendedPowerUpSpawnTimer = sf::Time::Zero;

            int powerUpType = m_extendedPowerUpDist(m_randomEngine);
            std::unique_ptr<PowerUp> powerUp;

            switch (powerUpType)
            {
            case 0: // Freeze
                powerUp = std::make_unique<FreezePowerUp>();
                if (auto* freeze = dynamic_cast<FreezePowerUp*>(powerUp.get()))
                {
                    freeze->setFont(getGame().getFonts().get(Fonts::Main));
                }
                break;
            case 1: // Extra Life
                powerUp = std::make_unique<ExtraLifePowerUp>();
                break;
            case 2: // Speed Boost
                powerUp = std::make_unique<SpeedBoostPowerUp>();
                break;
            }

            if (powerUp)
            {
                float x = std::uniform_real_distribution<float>(100.0f, 1820.0f)(m_randomEngine);
                float y = std::uniform_real_distribution<float>(100.0f, 980.0f)(m_randomEngine);
                powerUp->setPosition(x, y);

                // Store base Y for bobbing
                powerUp->m_baseY = y;

                m_bonusItems.push_back(std::move(powerUp));
            }
        }
    }

    void PlayState::checkCollisions()
    {
        // Player vs entities
        checkCollisionsWithContainer(*m_player, m_entities,
            [this](Entity& entity) { handleFishCollision(entity); });

        // Player vs fixed oysters
        m_oysterManager->checkCollisions(*m_player,
            [this](PermanentOyster* oyster) { handleOysterCollision(oyster); });

        // Player vs bonus items
        checkCollisionsWithContainer(*m_player, m_bonusItems,
            [this](BonusItem& item) { handleBonusItemCollision(item); });

        // Use new template-based fish collision system
        FishCollisionHandler::processFishCollisions(m_entities);

        // Process fish-to-hazard collisions
        FishCollisionHandler::processFishHazardCollisions(m_entities, m_hazards);

        // Process bomb explosions
        processBombExplosions(m_entities, m_hazards);

        // Fish-to-fish collisions
        for (size_t i = 0; i < m_entities.size(); ++i)
        {
            Fish* fish1 = dynamic_cast<Fish*>(m_entities[i].get());
            if (!fish1 || !fish1->isAlive()) continue;

            for (size_t j = i + 1; j < m_entities.size(); ++j)
            {
                Fish* fish2 = dynamic_cast<Fish*>(m_entities[j].get());
                if (!fish2 || !fish2->isAlive()) continue;

                checkCollisionPair(*fish1, *fish2,
                    [this](Fish& f1, Fish& f2) {
                        if (f1.canEat(f2))
                        {
                            f2.destroy();
                            createParticleEffect(f2.getPosition(), Constants::DEATH_PARTICLE_COLOR);
                        }
                        else if (f2.canEat(f1))
                        {
                            f1.destroy();
                            createParticleEffect(f1.getPosition(), Constants::DEATH_PARTICLE_COLOR);
                        }
                    });
            }
        }

        // Enemy fish vs oysters
        std::for_each(m_entities.begin(), m_entities.end(),
            [this](auto& entity) {
                if (Fish* fish = dynamic_cast<Fish*>(entity.get()))
                {
                    m_oysterManager->checkCollisions(*fish,
                        [this, &fish](PermanentOyster* oyster) {
                            if (oyster->canDamagePlayer())
                            {
                                fish->destroy();
                                createParticleEffect(fish->getPosition(), Constants::DEATH_PARTICLE_COLOR);
                                createParticleEffect(oyster->getPosition(), Constants::OYSTER_IMPACT_COLOR);
                            }
                        });
                }
            });

        // Check tail-bite opportunities
        std::for_each(m_entities.begin(), m_entities.end(),
            [this](auto& entity) {
                if (m_player->attemptTailBite(*entity))
                {
                    createParticleEffect(m_player->getPosition(), Constants::TAILBITE_PARTICLE_COLOR);
                }
            });
    }

    void PlayState::checkHazardCollisions()
    {
        // Player vs hazards
        checkCollisionsWithContainer(*m_player, m_hazards,
            [this](Hazard& hazard) { handleHazardCollision(hazard); });

        // Check bomb explosions vs entities
        std::for_each(m_hazards.begin(), m_hazards.end(),
            [this](auto& hazard) {
                if (Bomb* bomb = dynamic_cast<Bomb*>(hazard.get()))
                {
                    if (bomb->isExploding())
                    {
                        // Check explosion radius against all entities
                        std::for_each(m_entities.begin(), m_entities.end(),
                            [bomb](auto& entity) {
                                float dist = CollisionDetector::getDistance(
                                    bomb->getPosition(), entity->getPosition());
                                if (dist < bomb->getExplosionRadius())
                                {
                                    entity->destroy();
                                }
                            });

                        // Check against player
                        float playerDist = CollisionDetector::getDistance(
                            bomb->getPosition(), m_player->getPosition());
                        if (playerDist < bomb->getExplosionRadius() && !m_player->isInvulnerable())
                        {
                            m_player->takeDamage();
                            handlePlayerDeath();
                        }
                    }
                }
            });
    }

    void PlayState::handleFishCollision(Entity& fish)
    {
        if (m_player->isInvulnerable())
            return;

        // Special fish handling using visitor pattern
        struct FishCollisionVisitor
        {
            PlayState* state;
            Entity& fish;

            void operator()(Pufferfish* puffer) {
                if (puffer->isInflated())
                {
                    // Handle inflated pufferfish - push player
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
                else
                {
                    // Handle non-inflated pufferfish - can be eaten
                    if (state->m_player->canEat(fish))
                    {
                        if (state->m_player->attemptEat(fish))
                        {
                            fish.destroy();
                            state->createParticleEffect(fish.getPosition(),
                                Constants::EAT_PARTICLE_COLOR);
                        }
                    }
                    else
                    {
                        // Pufferfish can eat player if it's larger
                        if (puffer->canEat(*state->m_player) && !state->m_player->hasRecentlyTakenDamage())
                        {
                            state->m_player->takeDamage();
                            state->createParticleEffect(state->m_player->getPosition(),
                                Constants::DAMAGE_PARTICLE_COLOR);
                            state->handlePlayerDeath();
                        }
                    }
                }
            }

            void operator()(Angelfish* angel) {
                if (state->m_player->canEat(fish))
                {
                    if (state->m_player->attemptEat(fish))
                    {
                        state->createParticleEffect(fish.getPosition(),
                            Constants::ANGELFISH_PARTICLE_COLOR,
                            Constants::ANGELFISH_PARTICLE_COUNT);
                        fish.destroy();
                    }
                }
            }


            void operator()(PoisonFish* poisonFish) {
                if (state->m_player->canEat(fish))
                {
                    if (state->m_player->attemptEat(fish))
                    {
                        // Apply poison effect
                        state->reverseControls();
                        state->m_controlReverseTimer = poisonFish->getPoisonDuration();

                        // Visual feedback
                        state->createParticleEffect(fish.getPosition(), sf::Color::Magenta, 15);
                        state->createParticleEffect(state->m_player->getPosition(), sf::Color::Magenta, 10);

                        fish.destroy();
                    }
                }
            }

            void operator()(Fish* regularFish) {
                bool playerCanEat = state->m_player->canEat(fish);
                bool fishCanEatPlayer = regularFish->canEat(*state->m_player);

                if (playerCanEat)
                {
                    if (state->m_player->attemptEat(fish))
                    {
                        fish.destroy();
                        state->createParticleEffect(fish.getPosition(),
                            Constants::EAT_PARTICLE_COLOR);
                    }
                }
                else if (fishCanEatPlayer && !state->m_player->hasRecentlyTakenDamage())
                {
                    state->m_player->takeDamage();
                    state->createParticleEffect(state->m_player->getPosition(),
                        Constants::DAMAGE_PARTICLE_COLOR);
                    state->handlePlayerDeath();
                }
            }
        } visitor{ this, fish };

        // Apply visitor pattern
        if (auto* puffer = dynamic_cast<Pufferfish*>(&fish))
            visitor(puffer);
        else if (auto* angel = dynamic_cast<Angelfish*>(&fish))
            visitor(angel);
        else if (auto* poison = dynamic_cast<PoisonFish*>(&fish))
            visitor(poison);
        else if (auto* regularFish = dynamic_cast<Fish*>(&fish))
            visitor(regularFish);
    }

    void PlayState::handleBonusItemCollision(BonusItem& item)
    {
        if (PearlOyster* oyster = dynamic_cast<PearlOyster*>(&item))
        {
            if (!oyster->isOpen())
                return;
        }

        item.onCollect();

        if (PowerUp* powerUp = dynamic_cast<PowerUp*>(&item))
        {
            handlePowerUpCollision(*powerUp);
        }
        else
        {
            int frenzyMultiplier = m_frenzySystem->getMultiplier();
            float powerUpMultiplier = m_powerUpManager->getScoreMultiplier();

            m_scoreSystem->addScore(ScoreEventType::BonusCollected, item.getPoints(),
                item.getPosition(), frenzyMultiplier, powerUpMultiplier);

            createParticleEffect(item.getPosition(), Constants::BONUS_PARTICLE_COLOR);
        }
    }

    void PlayState::handlePowerUpCollision(PowerUp& powerUp)
    {
        // Power-up handling using map dispatch
        static const std::unordered_map<PowerUpType, std::function<void(PlayState*, PowerUp&)>> powerUpHandlers = {
            {PowerUpType::ScoreDoubler, [](PlayState* state, PowerUp& pu) {
                state->m_powerUpManager->activatePowerUp(pu.getPowerUpType(), pu.getDuration());
                state->createParticleEffect(pu.getPosition(), Constants::SCORE_DOUBLER_COLOR);
            }},
            {PowerUpType::FrenzyStarter, [](PlayState* state, PowerUp& pu) {
                state->m_frenzySystem->forceFrenzy();
                state->createParticleEffect(pu.getPosition(), Constants::FRENZY_STARTER_COLOR);
            }},
            {PowerUpType::SpeedBoost, [](PlayState* state, PowerUp& pu) {
                state->m_powerUpManager->activatePowerUp(pu.getPowerUpType(), pu.getDuration());
                state->m_player->applySpeedBoost(state->m_powerUpManager->getSpeedMultiplier(), pu.getDuration());
                state->createParticleEffect(pu.getPosition(), Constants::SPEED_BOOST_COLOR);
            }},
            {PowerUpType::Invincibility, [](PlayState* state, PowerUp& pu) {
                state->m_player->applyInvincibility(pu.getDuration());
                state->createParticleEffect(pu.getPosition(), Constants::INVINCIBILITY_COLOR);
            }},
            {PowerUpType::Freeze, [](PlayState* state, PowerUp& pu) {
                state->m_powerUpManager->activatePowerUp(pu.getPowerUpType(), pu.getDuration());
                state->applyFreeze();
                state->createParticleEffect(pu.getPosition(), sf::Color::Cyan, 20);
            }},
            {PowerUpType::ExtraLife, [](PlayState* state, PowerUp& pu) {
                state->m_gameState.playerLives++;
                state->createParticleEffect(pu.getPosition(), sf::Color::Green, 15);
            }},
            {PowerUpType::Shield, [](PlayState* state, PowerUp& pu) {
                state->m_powerUpManager->activatePowerUp(pu.getPowerUpType(), pu.getDuration());
                state->m_player->applyInvincibility(pu.getDuration());
                state->createParticleEffect(pu.getPosition(), sf::Color(255, 215, 0), 20);
            }}
        };

        if (auto it = powerUpHandlers.find(powerUp.getPowerUpType()); it != powerUpHandlers.end())
        {
            it->second(this, powerUp);
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

    void PlayState::handleHazardCollision(Hazard& hazard)
    {
        if (m_player->isInvulnerable())
            return;

        switch (hazard.getHazardType())
        {
        case HazardType::Bomb:
            if (Bomb* bomb = dynamic_cast<Bomb*>(&hazard))
            {
                bomb->onContact(*m_player);
                m_player->takeDamage();
                handlePlayerDeath();
                createParticleEffect(m_player->getPosition(), sf::Color::Red, 20);
            }
            break;

        case HazardType::Jellyfish:
            if (Jellyfish* jelly = dynamic_cast<Jellyfish*>(&hazard))
            {
                jelly->onContact(*m_player);
                m_isPlayerStunned = true;
                m_stunTimer = jelly->getStunDuration();
                m_player->setVelocity(0.0f, 0.0f);
                createParticleEffect(m_player->getPosition(), sf::Color(255, 255, 0, 150), 10);
            }
            break;
        }
    }

    void PlayState::applyFreeze()
    {
        m_isPlayerFrozen = true;
        m_freezeTimer = sf::seconds(5.0f);

        // Slow down all entities
        std::for_each(m_entities.begin(), m_entities.end(),
            [](auto& entity) {
                if (Fish* fish = dynamic_cast<Fish*>(entity.get()))
                {
                    fish->setVelocity(fish->getVelocity() * 0.1f); // 90% speed reduction
                }
            });
    }

    void PlayState::reverseControls()
    {
        m_hasControlsReversed = true;
        // Control reversal is handled in handleEvent()
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

        // Disable spawning
        m_fishSpawner->setLevel(-1);
        m_bonusItemManager->setStarfishEnabled(false);
        m_bonusItemManager->setPowerUpsEnabled(false);
    }

    void PlayState::makeAllEnemiesFlee()
    {
        std::for_each(m_entities.begin(), m_entities.end(),
            [](auto& entity) {
                if (Fish* fish = dynamic_cast<Fish*>(entity.get()))
                {
                    fish->startFleeing();
                }
            });
    }

    bool PlayState::areAllEnemiesGone() const
    {
        return std::none_of(m_entities.begin(), m_entities.end(),
            [](const auto& entity) {
                return entity->isAlive() &&
                    dynamic_cast<const Fish*>(entity.get()) != nullptr;
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

        // Change environment every few levels
        if (m_gameState.currentLevel % 3 == 0)
        {
            EnvironmentType newEnv = static_cast<EnvironmentType>(
                (static_cast<int>(m_environmentSystem->getCurrentEnvironment()) + 1) % 3);
            m_environmentSystem->setEnvironment(newEnv);
        }

        // Set random time of day for the new level
        m_environmentSystem->setRandomTimeOfDay();

        resetLevel();
        updateLevelDifficulty();

        m_hud.messageText.setString("");
        m_bonusStageTriggered = false;
    }

    void PlayState::resetLevel()
    {
        // Reset player
        m_player->fullReset();

        // Reset game state
        m_gameState.levelComplete = false;
        m_gameState.gameWon = false;
        m_gameState.enemiesFleeing = false;
        m_gameState.levelTime = sf::Time::Zero;

        // Clear containers
        m_entities.clear();
        m_bonusItems.clear();
        m_hazards.clear();
        m_particles.clear();

        // Reset systems
        m_scoreSystem->reset();
        m_frenzySystem->reset();
        m_powerUpManager->reset();
        m_growthMeter->reset();
        m_oysterManager->resetAll();

        // Reset effect states
        m_isPlayerFrozen = false;
        m_hasControlsReversed = false;
        m_isPlayerStunned = false;
        m_controlReverseTimer = sf::Time::Zero;
        m_freezeTimer = sf::Time::Zero;
        m_stunTimer = sf::Time::Zero;

        // Re-enable spawning
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

        // Calculate difficulty multipliers
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
        // Check if we should trigger bonus stage after completing a regular level
        if (!m_bonusStageTriggered && m_gameState.levelComplete && !m_returningFromBonusStage)
        {
            // Check if this is a 3rd, 6th, 9th, etc. level
            if (m_gameState.currentLevel % 3 == 0)
            {
                m_bonusStageTriggered = true;
                m_savedLevel = m_gameState.currentLevel; // Save current level

                // Determine bonus stage type
                BonusStageType bonusType = static_cast<BonusStageType>(
                    std::uniform_int_distribution<int>(0, 2)(m_randomEngine));

                // Push bonus stage
                deferAction([this, bonusType]() {
                    m_returningFromBonusStage = true; // Set flag before pushing
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
        // Use string stream for efficient string building
        auto formatText = [](sf::Text& text, const auto&... args) {
            std::ostringstream stream;
            ((stream << args), ...);
            text.setString(stream.str());
            };

        formatText(m_hud.scoreText,
            "Score: ", m_scoreSystem->getCurrentScore(),
            " | Points: ", m_player->getPoints(), "/", Constants::POINTS_TO_WIN);

        formatText(m_hud.livesText, "Lives: ", m_gameState.playerLives);

        // Updated level text formatting for bonus stage indication
        int levelsUntilBonus = 3 - (m_gameState.currentLevel % 3);
        if (levelsUntilBonus == 3) levelsUntilBonus = 0; // Just completed bonus stage cycle

        formatText(m_hud.levelText,
            "Level: ", m_gameState.currentLevel,
            " | Stage: ", m_player->getCurrentStage(), "/", Constants::MAX_STAGES,
            m_gameState.gameWon ? " | COMPLETE!" : "",
            levelsUntilBonus > 0 ? " | Bonus in: " : " | Bonus after this level!",
            levelsUntilBonus > 0 ? std::to_string(levelsUntilBonus) + " levels" : "");

        // Chain display
        if (m_scoreSystem->getChainBonus() > 0)
        {
            formatText(m_hud.chainText, "Chain Bonus: +", m_scoreSystem->getChainBonus());
        }
        else
        {
            m_hud.chainText.setString("");
        }

        // Power-up display
        auto activePowerUps = m_powerUpManager->getActivePowerUps();
        if (!activePowerUps.empty())
        {
            std::ostringstream powerUpStream;
            powerUpStream << "Active Power-Ups:\n";

            std::for_each(activePowerUps.begin(), activePowerUps.end(),
                [this, &powerUpStream](PowerUpType type) {
                    static const std::unordered_map<PowerUpType, std::string> powerUpNames = {
                        {PowerUpType::ScoreDoubler, "2X Score"},
                        {PowerUpType::SpeedBoost, "Speed Boost"},
                        {PowerUpType::Invincibility, "Invincible"},
                        {PowerUpType::Freeze, "Freeze"},
                        {PowerUpType::Shield, "Shield"}
                    };

                    if (auto it = powerUpNames.find(type); it != powerUpNames.end())
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

        // Environment info
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

        // Effects info
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

        // Draw environment first
        window.draw(*m_environmentSystem);

        // Render hazards
        std::for_each(m_hazards.begin(), m_hazards.end(),
            [&window](const auto& hazard) { window.draw(*hazard); });

        // Render all entities
        std::for_each(m_entities.begin(), m_entities.end(),
            [&window](const auto& entity) { window.draw(*entity); });

        // Render systems
        m_oysterManager->draw(window);

        // Render bonus items
        std::for_each(m_bonusItems.begin(), m_bonusItems.end(),
            [&window](const auto& item) { window.draw(*item); });

        // Render player
        window.draw(*m_player);

        // Render particles
        std::for_each(m_particles.begin(), m_particles.end(),
            [&window](const ParticleEffect& particle) { window.draw(particle.shape); });

        // Render UI systems
        m_scoreSystem->drawFloatingScores(window);
        window.draw(*m_growthMeter);
        window.draw(*m_frenzySystem);

        // Render HUD
        window.draw(m_hud.scoreText);
        window.draw(m_hud.livesText);
        window.draw(m_hud.levelText);
        window.draw(m_hud.chainText);
        window.draw(m_hud.powerUpText);
        window.draw(m_hud.fpsText);
        window.draw(m_hud.environmentText);
        window.draw(m_hud.effectsText);

        // Render overlay and message if needed
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
        // Check if we're returning from a bonus stage
        if (m_returningFromBonusStage)
        {
            // Don't reset everything - just prepare for next level
            m_returningFromBonusStage = false;
            m_bonusStageTriggered = false;

            // Advance to next level after bonus stage
            m_gameState.currentLevel = m_savedLevel + 1;
            m_gameState.levelComplete = false;
            m_gameState.gameWon = false;
            m_gameState.enemiesFleeing = false;
            m_gameState.levelTime = sf::Time::Zero;

            // Reset level-specific items but keep progress
            resetLevel();
            updateLevelDifficulty();

            // Clear any messages
            m_hud.messageText.setString("");
        }
        else
        {
            // Normal activation - starting fresh
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