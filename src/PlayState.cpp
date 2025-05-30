#include "PlayState.h"
#include "Game.h"
#include "CollisionDetector.h"
#include "Fish.h"
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
        , m_fishSpawner(std::make_unique<FishSpawner>(getGame().getWindow().getSize()))
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
        , m_levelStats()
        , m_fpsUpdateTime(sf::Time::Zero)
        , m_frameCount(0)
        , m_currentFPS(0.0f)
        , m_randomEngine(std::random_device{}())
    {
        auto& window = getGame().getWindow();
        auto& font = getGame().getFonts().get(Fonts::Main);

        // Initialize player with systems
        m_player->setWindowBounds(window.getSize());
        m_player->initializeSystems(m_growthMeter.get(), m_frenzySystem.get(),
            m_powerUpManager.get(), m_scoreSystem.get());

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

        // Handle level transition
        if (m_levelComplete)
        {
            m_levelTransitionTimer += deltaTime;
            if (m_levelTransitionTimer >= m_levelTransitionDuration)
            {
                advanceLevel();
            }
            return false; // Don't update game during transition
        }

        // Update game systems
        updateGameplay(deltaTime);

        return false; // Don't update underlying states
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

        // Draw bonus items (excluding oysters)
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

        // Draw message if level complete
        if (m_levelComplete)
        {
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

        // Update fish spawner
        m_fishSpawner->update(deltaTime, m_currentLevel);

        // Get newly spawned fish
        auto& spawnedFish = m_fishSpawner->getSpawnedFish();
        std::move(spawnedFish.begin(), spawnedFish.end(), std::back_inserter(m_entities));
        m_fishSpawner->clearSpawnedFish();

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

        // Check for level completion - NOW BASED ON REACHING STAGE 4
        if (m_player->getCurrentStage() >= 4)
        {
            completeLevel();
        }
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
        // This method can be used for additional power-up related updates if needed
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

        // Check player collisions with fixed oysters using template-based collision checking
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
                    // Oyster is open - player eats it for growth points
                    oyster->onCollect();

                    // Add growth points directly
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
    }

    void PlayState::handleFishCollision(Entity& fish)
    {
        // First check if player can eat the fish
        if (m_player->canEat(fish))
        {
            if (m_player->attemptEat(fish))
            {
                fish.destroy();
                createParticleEffect(fish.getPosition(), sf::Color::Green);
            }
        }
        else
        {
            // Fish is larger - check if it can eat the player
            if (Fish* predator = dynamic_cast<Fish*>(&fish))
            {
                if (predator->getSize() > m_player->getCurrentFishSize())
                {
                    // Player should take damage from larger fish
                    if (!m_player->isInvulnerable() && !m_player->hasRecentlyTakenDamage())
                    {
                        m_player->takeDamage();

                        // Check if player should die
                        if (!m_player->isInvulnerable()) // If not respawning
                        {
                            handlePlayerDeath();
                        }
                    }
                }
            }
        }
    }

    void PlayState::handleBonusItemCollision(BonusItem& item)
    {
        // Check specific bonus item types
        if (PearlOyster* oyster = dynamic_cast<PearlOyster*>(&item))
        {
            if (!oyster->isOpen())
                return; // Can't collect closed oyster
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
            createParticleEffect(powerUp.getPosition(), sf::Color(255, 215, 0)); // Gold
            break;
        }
    }

    void PlayState::checkTailBiteOpportunities()
    {
        // Find large fish that player can tail-bite
        std::for_each(m_entities.begin(), m_entities.end(),
            [this](std::unique_ptr<Entity>& entity) {
                if (m_player->attemptTailBite(*entity))
                {
                    createParticleEffect(m_player->getPosition(), sf::Color::Magenta);
                }
            });
    }

    void PlayState::handlePlayerDeath()
    {
        m_playerLives--;

        if (m_playerLives <= 0)
        {
            gameOver();
        }
        else
        {
            m_player->die();
            createParticleEffect(m_player->getPosition(), sf::Color::Red);
        }
    }

    void PlayState::completeLevel()
    {
        m_levelComplete = true;
        m_levelTransitionTimer = sf::Time::Zero;

        // Calculate level stats
        m_levelStats.completionTime = m_levelTime;
        m_levelStats.reachedMaxSize = m_player->isAtMaxSize();
        m_levelStats.tookNoDamage = !m_player->hasTakenDamage();

        // Calculate bonuses
        m_levelStats.timeBonus = m_scoreSystem->calculateTimeBonus(m_levelTime, m_targetLevelTime);
        m_levelStats.growthBonus = m_scoreSystem->calculateGrowthBonus(m_levelStats.reachedMaxSize);
        m_levelStats.untouchableBonus = m_scoreSystem->calculateUntouchableBonus(m_levelStats.tookNoDamage);
        m_levelStats.totalBonus = m_levelStats.timeBonus + m_levelStats.growthBonus + m_levelStats.untouchableBonus;

        // Add bonuses to score
        m_scoreSystem->addScore(ScoreEventType::LevelComplete, m_levelStats.totalBonus,
            sf::Vector2f(getGame().getWindow().getSize().x / 2.0f,
                getGame().getWindow().getSize().y / 2.0f),
            1, 1.0f);

        showEndOfLevelStats();
    }

    void PlayState::showEndOfLevelStats()
    {
        // Create level complete message with stats
        std::ostringstream messageStream;
        messageStream << "Stage 4 Reached! Level " << m_currentLevel << " Complete!\n\n"
            << "Time: " << std::fixed << std::setprecision(1)
            << m_levelTime.asSeconds() << "s\n"
            << "Fish Eaten: " << m_player->getTotalFishEaten() << "\n"
            << "Time Bonus: " << m_levelStats.timeBonus << "\n"
            << "Untouchable Bonus: " << m_levelStats.untouchableBonus << "\n"
            << "Total Bonus: " << m_levelStats.totalBonus;

        m_messageText.setString(messageStream.str());

        // Center the message
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
        m_player->resetSize();
        m_levelComplete = false;
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

        // Update difficulty
        m_fishSpawner->setLevel(m_currentLevel);
        m_bonusItemManager->setLevel(m_currentLevel);
        updateLevelDifficulty();
    }

    void PlayState::gameOver()
    {
        // Restart the current level
        m_playerLives = 3;
        m_player->resetSize();
        m_entities.clear();
        m_bonusItems.clear();
        m_particles.clear();

        // Reset systems
        m_scoreSystem->reset();
        m_frenzySystem->reset();
        m_powerUpManager->reset();
        m_growthMeter->reset();

        m_levelTime = sf::Time::Zero;
        m_levelStartTime = sf::Time::Zero;

        // Display game over message
        m_messageText.setString("Game Over! Restarting Level " + std::to_string(m_currentLevel));
        sf::FloatRect bounds = m_messageText.getLocalBounds();
        m_messageText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        m_messageText.setPosition(getGame().getWindow().getSize().x / 2.0f,
            getGame().getWindow().getSize().y / 2.0f);
    }

    void PlayState::updateLevelDifficulty()
    {
        // Update bonus item manager for new level
        m_bonusItemManager->setLevel(m_currentLevel);
    }

    void PlayState::createParticleEffect(sf::Vector2f position, sf::Color color)
    {
        // Create multiple particles using STL algorithms
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

    void PlayState::updateHUD()
    {
        // Update score display - show growth progress instead of score target
        std::ostringstream scoreStream;
        scoreStream << "Growth: " << std::fixed << std::setprecision(0)
            << m_growthMeter->getCurrentProgress() << "/"
            << m_growthMeter->getMaxProgress();
        m_scoreText.setString(scoreStream.str());

        // Update lives display
        std::ostringstream livesStream;
        livesStream << "Lives: " << m_playerLives;
        m_livesText.setString(livesStream.str());

        // Update level display with stage information
        std::ostringstream levelStream;
        levelStream << "Level: " << m_currentLevel
            << " | Stage: " << m_player->getCurrentStage() << "/4";
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

        // Update power-up display using STL algorithms
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