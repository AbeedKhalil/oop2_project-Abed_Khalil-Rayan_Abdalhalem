#include "PlayState.h"
#include "Game.h"
#include "CollisionDetector.h"
#include "Fish.h"
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
        , m_metrics()
        , m_particles()
        , m_randomEngine(std::random_device{}())
        , m_angleDist(0.0f, 360.0f)
        , m_speedDist(Constants::MIN_PARTICLE_SPEED, Constants::MAX_PARTICLE_SPEED)
    {
        initializeSystems();

        // Reserve capacity for containers
        m_entities.reserve(Constants::MAX_ENTITIES);
        m_bonusItems.reserve(Constants::MAX_BONUS_ITEMS);
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
        switch (event.type)
        {
        case sf::Event::KeyPressed:
            switch (event.key.code)
            {
            case sf::Keyboard::Escape:
                deferAction([this]() {
                    requestStackPop();
                    requestStackPush(StateID::Menu);
                    });
                break;

            case sf::Keyboard::Enter:
                if (m_gameState.levelComplete && !m_gameState.enemiesFleeing)
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
            m_player->followMouse(sf::Vector2f(
                static_cast<float>(event.mouseMove.x),
                static_cast<float>(event.mouseMove.y)
            ));
            break;

        default:
            break;
        }
    }

    bool PlayState::update(sf::Time deltaTime)
    {
        // Update performance metrics
        updatePerformanceMetrics(deltaTime);

        // Don't update gameplay if waiting for level transition
        if (m_gameState.levelComplete && !m_gameState.enemiesFleeing)
        {
            return false;
        }

        // Main gameplay update
        updateGameplay(deltaTime);

        // Process deferred actions
        processDeferredActions();

        return false;
    }

    void PlayState::updateGameplay(sf::Time deltaTime)
    {
        m_gameState.levelTime += deltaTime;

        // Update all systems
        updateSystems(deltaTime);

        // Check win condition
        if (!m_gameState.gameWon && !m_gameState.levelComplete)
        {
            checkWinCondition();
        }
        else if (m_gameState.gameWon)
        {
            m_gameState.winTimer += deltaTime;
            if (m_gameState.enemiesFleeing && areAllEnemiesGone())
            {
                m_gameState.enemiesFleeing = false;
                showMessage("Level Complete!\nPress ENTER to continue");
                m_gameState.levelComplete = true;
            }
        }

        // Update entities
        updateEntities(deltaTime);

        // Handle spawning (unless game is won)
        if (!m_gameState.gameWon)
        {
            m_fishSpawner->update(deltaTime, m_gameState.currentLevel);
            auto& spawnedFish = m_fishSpawner->getSpawnedFish();
            processSpawnedEntities(spawnedFish);
        }

        // Update bonus items
        m_bonusItemManager->update(deltaTime);
        auto newItems = m_bonusItemManager->collectSpawnedItems();
        std::move(newItems.begin(), newItems.end(), std::back_inserter(m_bonusItems));

        // Update containers
        updateContainer(m_bonusItems, deltaTime);
        updateContainer(m_entities, deltaTime);

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
        removeDeadEntities(m_particles, [](const auto& particle) {
            return particle.lifetime <= sf::Time::Zero;
            });

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
        m_player->update(deltaTime);
    }

    void PlayState::updateEntities(sf::Time deltaTime)
    {
        // Update all entities and their AI
        std::for_each(std::execution::par_unseq, m_entities.begin(), m_entities.end(),
            [this, deltaTime](const auto& entity) {
                entity->update(deltaTime);

                if (Fish* fish = dynamic_cast<Fish*>(entity.get()))
                {
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
                state->m_player->applySpeedBoost(Constants::SPEED_BOOST_MULTIPLIER, pu.getDuration());
                state->createParticleEffect(pu.getPosition(), Constants::SPEED_BOOST_COLOR);
            }},
            {PowerUpType::Invincibility, [](PlayState* state, PowerUp& pu) {
                state->m_player->applyInvincibility(pu.getDuration());
                state->createParticleEffect(pu.getPosition(), Constants::INVINCIBILITY_COLOR);
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

        resetLevel();
        updateLevelDifficulty();

        m_hud.messageText.setString("");
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
        m_particles.clear();

        // Reset systems
        m_scoreSystem->reset();
        m_frenzySystem->reset();
        m_powerUpManager->reset();
        m_growthMeter->reset();
        m_oysterManager->resetAll();

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

        formatText(m_hud.levelText,
            "Level: ", m_gameState.currentLevel,
            " | Stage: ", m_player->getCurrentStage(), "/", Constants::MAX_STAGES,
            m_gameState.gameWon ? " | COMPLETE!" : "");

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
                        {PowerUpType::Invincibility, "Invincible"}
                    };

                    if (auto it = powerUpNames.find(type); it != powerUpNames.end())
                    {
                        powerUpStream << it->second;
                        if (type == PowerUpType::ScoreDoubler)
                        {
                            powerUpStream << " - " << std::fixed << std::setprecision(1)
                                << m_powerUpManager->getRemainingTime(type).asSeconds() << "s";
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
        // Reset state when becoming active
        resetLevel();
        m_gameState.currentLevel = 1;
        m_gameState.playerLives = Constants::INITIAL_LIVES;
        m_gameState.totalScore = 0;
    }
}