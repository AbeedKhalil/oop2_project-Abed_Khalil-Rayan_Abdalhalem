#include "PlayState.h"
#include "Game.h"
#include "CollisionDetector.h"
#include "Fish.h"
#include "GameConstants.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <numeric>

namespace FishGame
{
    // Static member initialization
    const sf::Time PlayState::m_levelTransitionDuration = sf::seconds(5.0f);
    const sf::Time PlayState::m_targetLevelTime = sf::seconds(120.0f);

    PlayState::PlayState(Game& game)
        : State(game)
        , m_player(std::make_unique<Player>())
        , m_fishSpawner(std::make_unique<EnhancedFishSpawner>(getGame().getWindow().getSize()))
        , m_schoolingSystem(std::make_unique<SchoolingSystem>())
        , m_entities()
        , m_bonusItems()
        , m_growthMeter(std::make_unique<GrowthMeter>(getGame().getFonts().get(Fonts::Main)))
        , m_frenzySystem(std::make_unique<FrenzySystem>(getGame().getFonts().get(Fonts::Main)))
        , m_powerUpManager(std::make_unique<PowerUpManager>())
        , m_scoreSystem(std::make_unique<ScoreSystem>(getGame().getFonts().get(Fonts::Main)))
        , m_bonusItemManager(std::make_unique<BonusItemManager>(getGame().getWindow().getSize(), getGame().getFonts().get(Fonts::Main)))
        , m_oysterManager(std::make_unique<FixedOysterManager>(getGame().getWindow().getSize()))
        , m_scoreText()
        , m_livesText()
        , m_levelText()
        , m_fpsText()
        , m_messageText()
        , m_chainText()
        , m_powerUpText()
        , m_currentLevel(1)
        , m_playerLives(3)
        , m_totalScore(0)
        , m_levelStartTime(sf::Time::Zero)
        , m_levelTime(sf::Time::Zero)
        , m_levelComplete(false)
        , m_levelTransitionTimer(sf::Time::Zero)
        , m_gameWon(false)
        , m_enemiesFleeing(false)
        , m_winTimer(sf::Time::Zero)
        , m_levelStats()
        , m_fpsUpdateTime(sf::Time::Zero)
        , m_frameCount(0)
        , m_currentFPS(0.0f)
        , m_particles()
        , m_randomEngine(std::random_device{}())
    {
        auto& window = getGame().getWindow();
        auto& font = getGame().getFonts().get(Fonts::Main);

        // Initialize player with systems
        m_player->setWindowBounds(window.getSize());
        m_player->initializeSystems(m_growthMeter.get(), m_frenzySystem.get(),
            m_powerUpManager.get(), m_scoreSystem.get());

        // Set up enhanced fish spawner with schooling system
        m_fishSpawner->setSchoolingSystem(m_schoolingSystem.get());

        // Configure special fish spawn rates based on level
        SpecialFishConfig specialConfig;
        specialConfig.barracudaSpawnRate = 0.05f;
        specialConfig.pufferfishSpawnRate = 0.08f;
        specialConfig.angelfishSpawnRate = 0.1f;
        specialConfig.schoolSpawnChance = 0.05f;  // Reduced from 0.1f to 0.05f - less frequent schools
        m_fishSpawner->setSpecialFishConfig(specialConfig);

        // Position game systems UI
        m_growthMeter->setPosition(20.0f, window.getSize().y - 60.0f);
        m_frenzySystem->setPosition(window.getSize().x / 2.0f, 100.0f);

        // Initialize HUD elements
        const float hudMargin = 20.0f;
        const unsigned int hudFontSize = 24;

        // Score text
        m_scoreText.setFont(font);
        m_scoreText.setCharacterSize(hudFontSize);
        m_scoreText.setFillColor(sf::Color::White);
        m_scoreText.setPosition(hudMargin, hudMargin);

        // Lives text
        m_livesText.setFont(font);
        m_livesText.setCharacterSize(hudFontSize);
        m_livesText.setFillColor(sf::Color::White);
        m_livesText.setPosition(hudMargin, hudMargin + 30.0f);

        // Level text
        m_levelText.setFont(font);
        m_levelText.setCharacterSize(hudFontSize);
        m_levelText.setFillColor(sf::Color::White);
        m_levelText.setPosition(hudMargin, hudMargin + 60.0f);

        // Chain text
        m_chainText.setFont(font);
        m_chainText.setCharacterSize(20);
        m_chainText.setFillColor(sf::Color::Cyan);
        m_chainText.setPosition(hudMargin, hudMargin + 90.0f);

        // Power-up text
        m_powerUpText.setFont(font);
        m_powerUpText.setCharacterSize(20);
        m_powerUpText.setFillColor(sf::Color::Yellow);
        m_powerUpText.setPosition(window.getSize().x - 300.0f, hudMargin + 30.0f);

        // FPS text
        m_fpsText.setFont(font);
        m_fpsText.setCharacterSize(hudFontSize);
        m_fpsText.setFillColor(sf::Color::Green);
        m_fpsText.setPosition(window.getSize().x - 100.0f, hudMargin);

        // Message text (centered)
        m_messageText.setFont(font);
        m_messageText.setCharacterSize(48);
        m_messageText.setFillColor(sf::Color::Yellow);
        m_messageText.setOutlineColor(sf::Color::Black);
        m_messageText.setOutlineThickness(2.0f);

        // Set fish spawner level
        m_fishSpawner->setLevel(m_currentLevel);

        // Disable oyster spawning in BonusItemManager since we have fixed oysters
        m_bonusItemManager->setOysterEnabled(false);

        // Reserve space for entities
        m_entities.reserve(50);
        m_bonusItems.reserve(10);
        m_particles.reserve(100);

        updateHUD();
    }

    void PlayState::handleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
            case sf::Keyboard::Escape:
                requestStackPop();
                requestStackPush(StateID::Menu);
                break;

            case sf::Keyboard::Enter:
                // Handle level advancement after bonus round
                if (m_levelComplete && !m_enemiesFleeing)
                {
                    advanceLevel();
                }
                break;

            case sf::Keyboard::P:
                // TODO: Implement pause state
                break;

            default:
                break;
            }
        }
        else if (event.type == sf::Event::MouseMoved)
        {
            // Update mouse position for player
            sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x),
                static_cast<float>(event.mouseMove.y));
            m_player->followMouse(mousePos);
        }
    }

    bool PlayState::update(sf::Time deltaTime)
    {
        // Update FPS counter
        m_frameCount++;
        m_fpsUpdateTime += deltaTime;
        if (m_fpsUpdateTime >= sf::seconds(1.0f))
        {
            m_currentFPS = static_cast<float>(m_frameCount) / m_fpsUpdateTime.asSeconds();
            m_frameCount = 0;
            m_fpsUpdateTime = sf::Time::Zero;

            std::ostringstream fpsStream;
            fpsStream << std::fixed << std::setprecision(1) << "FPS: " << m_currentFPS;
            m_fpsText.setString(fpsStream.str());
        }

        // Don't update gameplay if waiting for level transition
        if (m_levelComplete && !m_enemiesFleeing)
        {
            return false;
        }

        // Update game systems
        updateGameplay(deltaTime);

        return false;
    }

    void PlayState::render()
    {
        auto& window = getGame().getWindow();

        // Draw all entities
        std::for_each(m_entities.begin(), m_entities.end(),
            [&window](const std::unique_ptr<Entity>& entity) {
                window.draw(*entity);
            });

        // Draw fixed oysters
        m_oysterManager->draw(window);

        // Draw bonus items
        std::for_each(m_bonusItems.begin(), m_bonusItems.end(),
            [&window](const std::unique_ptr<BonusItem>& item) {
                window.draw(*item);
            });

        // Draw player
        window.draw(*m_player);

        // Draw particle effects
        std::for_each(m_particles.begin(), m_particles.end(),
            [&window](const ParticleEffect& particle) {
                window.draw(particle.shape);
            });

        // Draw floating scores
        m_scoreSystem->drawFloatingScores(window);

        // Draw game systems UI
        window.draw(*m_growthMeter);
        window.draw(*m_frenzySystem);

        // Draw HUD
        window.draw(m_scoreText);
        window.draw(m_livesText);
        window.draw(m_levelText);
        window.draw(m_chainText);
        window.draw(m_powerUpText);
        window.draw(m_fpsText);

        // Draw message if game won or level complete
        if (m_gameWon || m_levelComplete)
        {
            // Draw semi-transparent overlay
            sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
            overlay.setFillColor(sf::Color(0, 0, 0, 128));
            window.draw(overlay);

            window.draw(m_messageText);
        }
    }

    void PlayState::updateGameplay(sf::Time deltaTime)
    {
        m_levelTime += deltaTime;

        // Update game systems
        m_frenzySystem->update(deltaTime);
        m_powerUpManager->update(deltaTime);
        m_scoreSystem->update(deltaTime);
        m_growthMeter->update(deltaTime);

        // Check win condition only if not already won
        if (!m_gameWon && !m_levelComplete)
        {
            checkWinCondition();
        }
        else if (m_gameWon)
        {
            // Update win timer
            m_winTimer += deltaTime;

            // Check if all enemies are gone
            if (m_enemiesFleeing && areAllEnemiesGone())
            {
                m_enemiesFleeing = false;
                showBonusPointsMessage();
                m_levelComplete = true;
            }
        }

        // Update schooling system
        updateSchoolingBehavior(deltaTime);

        // Update fish spawner (unless game is won)
        if (!m_gameWon)
        {
            m_fishSpawner->update(deltaTime, m_currentLevel);

            // Get newly spawned fish
            auto& spawnedFish = m_fishSpawner->getSpawnedFish();
            std::move(spawnedFish.begin(), spawnedFish.end(), std::back_inserter(m_entities));
            m_fishSpawner->clearSpawnedFish();
        }

        // Update bonus items
        updateBonusItems(deltaTime);

        // Update power-ups
        updatePowerUps(deltaTime);

        // Update fixed oysters
        m_oysterManager->update(deltaTime);

        // Update player
        m_player->update(deltaTime);

        // Update all entities and their AI
        std::for_each(m_entities.begin(), m_entities.end(),
            [this, deltaTime](const std::unique_ptr<Entity>& entity) {
                entity->update(deltaTime);

                // Update fish AI
                if (Fish* fish = dynamic_cast<Fish*>(entity.get()))
                {
                    fish->updateAI(m_entities, m_player.get(), deltaTime);
                }
            });

        // Handle special fish behaviors
        handleSpecialFishBehaviors(deltaTime);

        // Update particles
        std::for_each(m_particles.begin(), m_particles.end(),
            [deltaTime](ParticleEffect& particle) {
                particle.lifetime -= deltaTime;
                particle.shape.move(particle.velocity * deltaTime.asSeconds());
                particle.alpha = std::max(0.0f, particle.alpha - 255.0f * deltaTime.asSeconds());
                sf::Color color = particle.shape.getFillColor();
                color.a = static_cast<sf::Uint8>(particle.alpha);
                particle.shape.setFillColor(color);
            });

        // Remove dead entities and expired particles using STL algorithms
        m_entities.erase(
            std::remove_if(m_entities.begin(), m_entities.end(),
                [](const std::unique_ptr<Entity>& entity) {
                    return !entity->isAlive();
                }),
            m_entities.end()
        );

        m_bonusItems.erase(
            std::remove_if(m_bonusItems.begin(), m_bonusItems.end(),
                [](const std::unique_ptr<BonusItem>& item) {
                    return !item->isAlive() || item->hasExpired();
                }),
            m_bonusItems.end()
        );

        m_particles.erase(
            std::remove_if(m_particles.begin(), m_particles.end(),
                [](const ParticleEffect& particle) {
                    return particle.lifetime <= sf::Time::Zero;
                }),
            m_particles.end()
        );

        // Check collisions
        checkCollisions();
        checkTailBiteOpportunities();

        // Update HUD
        updateHUD();
    }

    void PlayState::updateBonusItems(sf::Time deltaTime)
    {
        // Update bonus item manager
        m_bonusItemManager->update(deltaTime);

        // Collect newly spawned items
        auto newItems = m_bonusItemManager->collectSpawnedItems();
        std::move(newItems.begin(), newItems.end(), std::back_inserter(m_bonusItems));

        // Update existing items
        std::for_each(m_bonusItems.begin(), m_bonusItems.end(),
            [deltaTime](const std::unique_ptr<BonusItem>& item) {
                item->update(deltaTime);
            });
    }

    void PlayState::updatePowerUps(sf::Time deltaTime)
    {
        // Power-ups are now handled by BonusItemManager
    }

    void PlayState::updateSchoolingBehavior(sf::Time deltaTime)
    {
        // Update schooling system
        m_schoolingSystem->update(deltaTime);

        // Periodically extract fish from schools for rendering
        static sf::Time extractTimer = sf::Time::Zero;
        extractTimer += deltaTime;

        if (extractTimer >= sf::seconds(0.1f))
        {
            extractTimer = sf::Time::Zero;

            // Get fish from schools
            auto schoolFish = m_schoolingSystem->extractAllFish();

            // Add them back to entities
            std::move(schoolFish.begin(), schoolFish.end(),
                std::back_inserter(m_entities));

            // Try to re-add fish to schools
            std::vector<std::unique_ptr<Entity>> remainingEntities;

            for (auto& entity : m_entities)
            {
                bool addedToSchool = false;

                // Only try to add small fish to schools  
                if (auto* smallFish = dynamic_cast<SmallFish*>(entity.get()))
                {
                    if (smallFish->isAlive())
                    {
                        auto schoolMember = std::make_unique<SmallFish>(m_currentLevel);
                        schoolMember->setPosition(smallFish->getPosition());
                        schoolMember->setVelocity(smallFish->getVelocity());
                        schoolMember->setWindowBounds(getGame().getWindow().getSize());

                        addedToSchool = m_schoolingSystem->tryAddToSchool(std::move(schoolMember));
                    }
                }

                if (!addedToSchool)
                {
                    remainingEntities.push_back(std::move(entity));
                }
            }

            m_entities = std::move(remainingEntities);
        }
    }

    void PlayState::handleSpecialFishBehaviors(sf::Time deltaTime)
    {
        // Handle Pufferfish threat detection and Angelfish AI
        std::for_each(m_entities.begin(), m_entities.end(),
            [this, deltaTime](const std::unique_ptr<Entity>& entity)  // Fixed: Added deltaTime to capture list
            {
                if (auto* pufferfish = dynamic_cast<Pufferfish*>(entity.get()))
                {
                    checkPufferfishThreat(pufferfish);
                }
                // Update Angelfish AI
                else if (auto* angelfish = dynamic_cast<Angelfish*>(entity.get()))
                {
                    angelfish->updateAI(m_entities, m_player.get(), deltaTime);
                }
            });
    }

    void PlayState::checkCollisions()
    {
        // Check player collisions with entities
        std::for_each(m_entities.begin(), m_entities.end(),
            [this](std::unique_ptr<Entity>& entity) {
                if (CollisionDetector::checkCircleCollision(*m_player, *entity))
                {
                    handleFishCollision(*entity);
                }
            });

        // Check player collisions with fixed oysters
        m_oysterManager->checkCollisions(*m_player,
            [this](PermanentOyster* oyster) {
                if (oyster->canDamagePlayer())
                {
                    // Oyster is closed - damage player
                    if (!m_player->isInvulnerable())
                    {
                        m_player->takeDamage();
                        handlePlayerDeath();
                        createParticleEffect(m_player->getPosition(), sf::Color::Red);
                    }
                }
                else if (oyster->canBeEaten())
                {
                    // Oyster is open - player eats it for points
                    oyster->onCollect();

                    // Add points based on oyster type
                    int points = oyster->hasBlackPearl() ? Constants::BLACK_OYSTER_POINTS : Constants::WHITE_OYSTER_POINTS;
                    m_player->addPoints(points);

                    // Add growth for visual effect
                    m_player->grow(oyster->getGrowthPoints());

                    // Add score with multipliers
                    int frenzyMultiplier = m_frenzySystem->getMultiplier();
                    float powerUpMultiplier = m_powerUpManager->getScoreMultiplier();

                    m_scoreSystem->addScore(ScoreEventType::BonusCollected, oyster->getPoints(),
                        oyster->getPosition(), frenzyMultiplier, powerUpMultiplier);

                    createParticleEffect(oyster->getPosition(),
                        oyster->hasBlackPearl() ? sf::Color::Magenta : sf::Color::White);
                }
            });

        // Check player collisions with bonus items
        std::for_each(m_bonusItems.begin(), m_bonusItems.end(),
            [this](std::unique_ptr<BonusItem>& item) {
                if (CollisionDetector::checkCircleCollision(*m_player, *item))
                {
                    handleBonusItemCollision(*item);
                }
            });

        // Check fish-to-fish collisions
        for (size_t i = 0; i < m_entities.size(); ++i)
        {
            Fish* fish1 = dynamic_cast<Fish*>(m_entities[i].get());
            if (!fish1 || !fish1->isAlive())
                continue;

            for (size_t j = i + 1; j < m_entities.size(); ++j)
            {
                Fish* fish2 = dynamic_cast<Fish*>(m_entities[j].get());
                if (!fish2 || !fish2->isAlive())
                    continue;

                if (CollisionDetector::checkCircleCollision(*fish1, *fish2))
                {
                    // Larger fish eats smaller fish
                    if (fish1->canEat(*fish2))
                    {
                        fish2->destroy();
                        createParticleEffect(fish2->getPosition(), sf::Color::Red);
                    }
                    else if (fish2->canEat(*fish1))
                    {
                        fish1->destroy();
                        createParticleEffect(fish1->getPosition(), sf::Color::Red);
                    }
                }
            }
        }

        // NEW: Check enemy fish collisions with fixed oysters
        std::for_each(m_entities.begin(), m_entities.end(),
            [this](std::unique_ptr<Entity>& entity) {
                if (Fish* fish = dynamic_cast<Fish*>(entity.get()))
                {
                    m_oysterManager->checkCollisions(*fish,
                        [this, &fish](PermanentOyster* oyster) {
                            if (oyster->canDamagePlayer()) // Closed oyster
                            {
                                // Enemy fish dies when trying to eat closed oyster
                                fish->destroy();
                                createParticleEffect(fish->getPosition(), sf::Color::Red);
                                createParticleEffect(oyster->getPosition(), sf::Color(100, 100, 100)); // Gray for oyster
                            }
                            // Enemy fish do NOT collect points from open oysters
                        });
                }
            });
    }

    void PlayState::handleFishCollision(Entity& fish)
    {
        // Skip if player is invulnerable
        if (m_player->isInvulnerable())
            return;

        // Determine if player can eat this fish
        bool playerCanEat = m_player->canEat(fish);

        // Determine if fish can eat player
        bool fishCanEatPlayer = false;
        if (Fish* fishPtr = dynamic_cast<Fish*>(&fish))
        {
            fishCanEatPlayer = fishPtr->canEat(*m_player);
        }

        // Handle special fish types first
        if (auto* pufferfish = dynamic_cast<Pufferfish*>(&fish))
        {
            if (pufferfish->isInflated())
            {
                // Player tries to eat inflated pufferfish
                if (!m_player->hasRecentlyTakenDamage())
                {
                    // Push player away
                    pufferfish->pushEntity(*m_player);

                    // Reduce score by 20 (but not below 0)
                    int currentScore = m_scoreSystem->getCurrentScore();
                    int newScore = std::max(0, currentScore - 20);
                    m_scoreSystem->setCurrentScore(newScore);

                    // Create visual feedback for score reduction
                    createScoreReductionEffect(m_player->getPosition(), -20);
                    createParticleEffect(m_player->getPosition(), sf::Color::Yellow);
                }
                return;
            }
            // Non-inflated pufferfish uses normal logic below
        }
        else if (auto* angelfish = dynamic_cast<Angelfish*>(&fish))
        {
            if (playerCanEat)
            {
                if (m_player->attemptEat(fish))
                {
                    createParticleEffect(fish.getPosition(), sf::Color::Cyan);
                    createParticleEffect(fish.getPosition(), sf::Color::Magenta);
                    createParticleEffect(fish.getPosition(), sf::Color::Yellow);
                    fish.destroy();
                }
            }
            return;
        }
        else if (auto* barracuda = dynamic_cast<Barracuda*>(&fish))
        {
            if (playerCanEat)
            {
                if (m_player->attemptEat(fish))
                {
                    fish.destroy();
                    createParticleEffect(fish.getPosition(), sf::Color::Green);
                }
            }
            else if (fishCanEatPlayer)
            {
                if (!m_player->hasRecentlyTakenDamage())
                {
                    m_player->takeDamage();
                    createParticleEffect(m_player->getPosition(), sf::Color::Red);
                    createParticleEffect(m_player->getPosition(), sf::Color(100, 0, 0));
                    handlePlayerDeath();
                }
            }
            return;
        }

        // Handle normal fish collision
        if (playerCanEat)
        {
            if (m_player->attemptEat(fish))
            {
                fish.destroy();
                createParticleEffect(fish.getPosition(), sf::Color::Green);
            }
        }
        else if (fishCanEatPlayer)
        {
            // Enemy fish eats player - player dies
            if (!m_player->hasRecentlyTakenDamage())
            {
                m_player->takeDamage();
                createParticleEffect(m_player->getPosition(), sf::Color::Red);
                handlePlayerDeath();
            }
        }
    }

    void PlayState::handleBonusItemCollision(BonusItem& item)
    {
        // Check specific bonus item types
        if (PearlOyster* oyster = dynamic_cast<PearlOyster*>(&item))
        {
            if (!oyster->isOpen())
                return;
        }

        // Collect the item
        item.onCollect();

        // Handle power-ups separately
        if (PowerUp* powerUp = dynamic_cast<PowerUp*>(&item))
        {
            handlePowerUpCollision(*powerUp);
        }
        else
        {
            // Regular bonus item - add score
            int frenzyMultiplier = m_frenzySystem->getMultiplier();
            float powerUpMultiplier = m_powerUpManager->getScoreMultiplier();

            m_scoreSystem->addScore(ScoreEventType::BonusCollected, item.getPoints(),
                item.getPosition(), frenzyMultiplier, powerUpMultiplier);

            createParticleEffect(item.getPosition(), sf::Color::Yellow);
        }
    }

    void PlayState::handlePowerUpCollision(PowerUp& powerUp)
    {
        PowerUpType type = powerUp.getPowerUpType();

        switch (type)
        {
        case PowerUpType::ScoreDoubler:
            m_powerUpManager->activatePowerUp(type, powerUp.getDuration());
            createParticleEffect(powerUp.getPosition(), sf::Color::Yellow);
            break;

        case PowerUpType::FrenzyStarter:
            m_frenzySystem->forceFrenzy();
            createParticleEffect(powerUp.getPosition(), sf::Color::Magenta);
            break;

        case PowerUpType::SpeedBoost:
            m_player->applySpeedBoost(1.5f, powerUp.getDuration());
            createParticleEffect(powerUp.getPosition(), sf::Color::Cyan);
            break;

        case PowerUpType::Invincibility:
            m_player->applyInvincibility(powerUp.getDuration());
            createParticleEffect(powerUp.getPosition(), sf::Color(255, 215, 0));
            break;
        }
    }

    void PlayState::checkTailBiteOpportunities()
    {
        std::for_each(m_entities.begin(), m_entities.end(),
            [this](std::unique_ptr<Entity>& entity) {
                if (m_player->attemptTailBite(*entity))
                {
                    createParticleEffect(m_player->getPosition(), sf::Color::Magenta);
                }
            });
    }

    void PlayState::checkPufferfishThreat(Pufferfish* pufferfish)
    {
        if (!pufferfish || !pufferfish->isAlive())
            return;

        // Check for inflated pufferfish pushing entities
        if (pufferfish->isInflated())
        {
            // Check player
            if (pufferfish->canPushEntity(*m_player))
            {
                // This is handled in handleFishCollision
            }

            // Check other fish
            std::for_each(m_entities.begin(), m_entities.end(),
                [pufferfish, this](std::unique_ptr<Entity>& entity)
                {
                    if (entity.get() != pufferfish && entity->isAlive())
                    {
                        if (pufferfish->canPushEntity(*entity))
                        {
                            // Push the entity away
                            pufferfish->pushEntity(*entity);

                            // Create visual effect
                            createParticleEffect(entity->getPosition(), sf::Color::Yellow);
                        }
                    }
                });
        }
    }

    void PlayState::handlePlayerDeath()
    {
        // Only handle death if player isn't invulnerable
        if (m_player->isInvulnerable())
            return;

        m_playerLives--;
        m_player->die();

        if (m_playerLives <= 0)
        {
            gameOver();
        }
        else
        {
            // Respawn the player after death
            m_player->respawn();
            createParticleEffect(m_player->getPosition(), sf::Color::Red);
        }
    }

    void PlayState::checkWinCondition()
    {
        // Win when player reaches 400 points
        if (m_player->getPoints() >= Constants::POINTS_TO_WIN)
        {
            triggerWinSequence();
        }
    }

    void PlayState::triggerWinSequence()
    {
        m_gameWon = true;
        m_enemiesFleeing = true;
        m_winTimer = sf::Time::Zero;

        makeAllEnemiesFlee();
        showWinMessage();

        // Disable spawning
        m_fishSpawner->setLevel(-1);
        m_bonusItemManager->setStarfishEnabled(false);
        m_bonusItemManager->setPowerUpsEnabled(false);
    }

    void PlayState::makeAllEnemiesFlee()
    {
        std::for_each(m_entities.begin(), m_entities.end(),
            [](std::unique_ptr<Entity>& entity)
            {
                if (Fish* fish = dynamic_cast<Fish*>(entity.get()))
                {
                    fish->startFleeing();
                }
            });
    }

    bool PlayState::areAllEnemiesGone() const
    {
        return std::none_of(m_entities.begin(), m_entities.end(),
            [](const std::unique_ptr<Entity>& entity)
            {
                return entity->isAlive() &&
                    dynamic_cast<const Fish*>(entity.get()) != nullptr;
            });
    }

    void PlayState::showWinMessage()
    {
        std::ostringstream messageStream;
        messageStream << "LEVEL COMPLETE!\n\n"
            << "Points: " << m_player->getPoints() << "\n"
            << "Score: " << m_scoreSystem->getCurrentScore() << "\n"
            << "Eat the fleeing fish for bonus points!";

        m_messageText.setString(messageStream.str());

        sf::FloatRect bounds = m_messageText.getLocalBounds();
        m_messageText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        m_messageText.setPosition(getGame().getWindow().getSize().x / 2.0f,
            getGame().getWindow().getSize().y / 2.0f - 100.0f);
    }

    void PlayState::showBonusPointsMessage()
    {
        std::ostringstream messageStream;
        messageStream << "LEVEL " << m_currentLevel << " COMPLETE!\n\n"
            << "Final Points: " << m_player->getPoints() << "\n"
            << "Final Score: " << m_scoreSystem->getCurrentScore() << "\n"
            << "Total Fish Eaten: " << m_player->getTotalFishEaten() << "\n\n"
            << "Press ENTER for Level " << (m_currentLevel + 1);

        m_messageText.setString(messageStream.str());

        sf::FloatRect bounds = m_messageText.getLocalBounds();
        m_messageText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        m_messageText.setPosition(getGame().getWindow().getSize().x / 2.0f,
            getGame().getWindow().getSize().y / 2.0f);
    }

    void PlayState::completeLevel()
    {
        m_levelComplete = true;
        m_levelTransitionTimer = sf::Time::Zero;

        m_levelStats.completionTime = m_levelTime;
        m_levelStats.reachedMaxSize = m_player->isAtMaxSize();
        m_levelStats.tookNoDamage = !m_player->hasTakenDamage();

        m_levelStats.timeBonus = m_scoreSystem->calculateTimeBonus(m_levelTime, m_targetLevelTime);
        m_levelStats.growthBonus = m_scoreSystem->calculateGrowthBonus(m_levelStats.reachedMaxSize);
        m_levelStats.untouchableBonus = m_scoreSystem->calculateUntouchableBonus(m_levelStats.tookNoDamage);
        m_levelStats.totalBonus = m_levelStats.timeBonus + m_levelStats.growthBonus + m_levelStats.untouchableBonus;

        m_scoreSystem->addScore(ScoreEventType::LevelComplete, m_levelStats.totalBonus,
            sf::Vector2f(getGame().getWindow().getSize().x / 2.0f,
                getGame().getWindow().getSize().y / 2.0f),
            1, 1.0f);

        showEndOfLevelStats();
    }

    void PlayState::showEndOfLevelStats()
    {
        std::ostringstream messageStream;
        messageStream << "Level " << m_currentLevel << " Complete!\n\n"
            << "Time: " << std::fixed << std::setprecision(1)
            << m_levelTime.asSeconds() << "s\n"
            << "Fish Eaten: " << m_player->getTotalFishEaten() << "\n"
            << "Points: " << m_player->getPoints() << "\n"
            << "Time Bonus: " << m_levelStats.timeBonus << "\n"
            << "Untouchable Bonus: " << m_levelStats.untouchableBonus << "\n"
            << "Total Bonus: " << m_levelStats.totalBonus;

        m_messageText.setString(messageStream.str());

        sf::FloatRect bounds = m_messageText.getLocalBounds();
        m_messageText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        m_messageText.setPosition(getGame().getWindow().getSize().x / 2.0f,
            getGame().getWindow().getSize().y / 2.0f);
    }

    void PlayState::advanceLevel()
    {
        m_currentLevel++;
        m_totalScore += m_scoreSystem->getCurrentScore();
        m_scoreSystem->addToTotalScore(m_scoreSystem->getCurrentScore());

        // Reset for new level
        m_player->fullReset();
        m_levelComplete = false;
        m_gameWon = false;
        m_enemiesFleeing = false;
        m_levelTime = sf::Time::Zero;
        m_levelStartTime = sf::Time::Zero;

        // Clear entities and items
        m_entities.clear();
        m_bonusItems.clear();
        m_particles.clear();

        // Reset systems
        m_scoreSystem->reset();
        m_frenzySystem->reset();
        m_powerUpManager->reset();
        m_growthMeter->reset();
        m_oysterManager->resetAll();

        // Update difficulty
        m_fishSpawner->setLevel(m_currentLevel);
        m_bonusItemManager->setLevel(m_currentLevel);
        updateLevelDifficulty();

        // Re-enable spawning
        m_bonusItemManager->setStarfishEnabled(true);
        m_bonusItemManager->setPowerUpsEnabled(true);

        // Clear messages
        m_messageText.setString("");
    }

    void PlayState::gameOver()
    {
        // Reset current level
        m_playerLives = 3;
        m_player->fullReset();
        m_entities.clear();
        m_bonusItems.clear();
        m_particles.clear();

        // Reset systems
        m_scoreSystem->reset();
        m_frenzySystem->reset();
        m_powerUpManager->reset();
        m_growthMeter->reset();
        m_oysterManager->resetAll();

        m_levelTime = sf::Time::Zero;
        m_levelStartTime = sf::Time::Zero;
        m_gameWon = false;
        m_enemiesFleeing = false;

        // Display game over message
        m_messageText.setString("Game Over! Restarting Level " + std::to_string(m_currentLevel));
        sf::FloatRect bounds = m_messageText.getLocalBounds();
        m_messageText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        m_messageText.setPosition(getGame().getWindow().getSize().x / 2.0f,
            getGame().getWindow().getSize().y / 2.0f);
    }

    void PlayState::updateLevelDifficulty()
    {
        m_bonusItemManager->setLevel(m_currentLevel);

        SpecialFishConfig specialConfig;

        // Base rates with controlled increments
        specialConfig.barracudaSpawnRate = 0.05f + (m_currentLevel - 1) * 0.01f;    // 0.05, 0.06, 0.07
        specialConfig.pufferfishSpawnRate = 0.15f + (m_currentLevel - 1) * 0.02f;   // Increased: 0.15, 0.17, 0.19
        specialConfig.angelfishSpawnRate = 0.02f + (m_currentLevel - 1) * 0.01f;    // Rare: 0.02, 0.03, 0.04

        // Keep school spawn chance low and controlled
        specialConfig.schoolSpawnChance = 0.05f + (m_currentLevel - 1) * 0.02f;     // 0.05, 0.07, 0.09

        // Cap the maximum school spawn chance
        specialConfig.schoolSpawnChance = std::min(0.10f, specialConfig.schoolSpawnChance);

        m_fishSpawner->setSpecialFishConfig(specialConfig);
        m_fishSpawner->setLevel(m_currentLevel);
    }

    void PlayState::createParticleEffect(sf::Vector2f position, sf::Color color)
    {
        std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
        std::uniform_real_distribution<float> speedDist(50.0f, 150.0f);

        constexpr int particleCount = 8;
        m_particles.reserve(m_particles.size() + particleCount);

        std::generate_n(std::back_inserter(m_particles), particleCount,
            [this, &position, &color, &angleDist, &speedDist]() {
                ParticleEffect particle;
                particle.shape = sf::CircleShape(3.0f);
                particle.shape.setFillColor(color);
                particle.shape.setPosition(position);

                float angle = angleDist(m_randomEngine) * 3.14159f / 180.0f;
                float speed = speedDist(m_randomEngine);
                particle.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

                particle.lifetime = sf::seconds(1.0f);
                particle.alpha = 255.0f;

                return particle;
            });
    }

    void PlayState::createScoreReductionEffect(sf::Vector2f position, int points)
    {
        // Create a temporary floating text effect for score reduction
        struct ScoreReduction
        {
            sf::Text text;
            sf::Vector2f position;
            float lifetime;
        };

        static std::vector<ScoreReduction> scoreReductions;

        ScoreReduction reduction;
        reduction.text.setFont(getGame().getFonts().get(Fonts::Main));
        reduction.text.setString(std::to_string(points));
        reduction.text.setCharacterSize(36);
        reduction.text.setFillColor(sf::Color::Red);
        reduction.text.setOutlineColor(sf::Color::Black);
        reduction.text.setOutlineThickness(2.0f);
        reduction.position = position;
        reduction.lifetime = 1.5f;

        // Center the text
        sf::FloatRect bounds = reduction.text.getLocalBounds();
        reduction.text.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        reduction.text.setPosition(position);

        // Create upward floating particles
        createParticleEffect(position, sf::Color::Red);
    }

    void PlayState::updateHUD()
    {
        // Update score display - show both Score and Points
        std::ostringstream scoreStream;
        scoreStream << "Score: " << m_scoreSystem->getCurrentScore();
        scoreStream << " | Points: " << m_player->getPoints() << "/" << Constants::POINTS_TO_WIN;
        m_scoreText.setString(scoreStream.str());

        // Update lives display
        std::ostringstream livesStream;
        livesStream << "Lives: " << m_playerLives;
        m_livesText.setString(livesStream.str());

        // Update level display with stage information
        std::ostringstream levelStream;
        levelStream << "Level: " << m_currentLevel
            << " | Stage: " << m_player->getCurrentStage() << "/" << Constants::MAX_STAGES;

        if (m_gameWon)
        {
            levelStream << " | COMPLETE!";
        }

        m_levelText.setString(levelStream.str());

        // Update chain display
        if (m_scoreSystem->getChainBonus() > 0)
        {
            std::ostringstream chainStream;
            chainStream << "Chain Bonus: +" << m_scoreSystem->getChainBonus();
            m_chainText.setString(chainStream.str());
        }
        else
        {
            m_chainText.setString("");
        }

        // Update power-up display
        auto activePowerUps = m_powerUpManager->getActivePowerUps();
        if (!activePowerUps.empty())
        {
            std::ostringstream powerUpStream;
            powerUpStream << "Active Power-Ups:\n";

            std::for_each(activePowerUps.begin(), activePowerUps.end(),
                [this, &powerUpStream](PowerUpType type) {
                    switch (type)
                    {
                    case PowerUpType::ScoreDoubler:
                        powerUpStream << "2X Score - "
                            << std::fixed << std::setprecision(1)
                            << m_powerUpManager->getRemainingTime(type).asSeconds() << "s\n";
                        break;
                    case PowerUpType::SpeedBoost:
                        powerUpStream << "Speed Boost\n";
                        break;
                    case PowerUpType::Invincibility:
                        powerUpStream << "Invincible\n";
                        break;
                    default:
                        break;
                    }
                });

            m_powerUpText.setString(powerUpStream.str());
        }
        else
        {
            m_powerUpText.setString("");
        }
    }
}