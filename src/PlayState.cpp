// PlayState.cpp
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
    using namespace Constants;

    // Static member initialization
    const sf::Time PlayState::m_levelTransitionDuration = LEVEL_TRANSITION_DURATION;

    PlayState::PlayState(Game& game)
        : State(game)
        , m_player(std::make_unique<Player>())
        , m_fishSpawner(std::make_unique<FishSpawner>(getGame().getWindow().getSize()))
        , m_entities()
        , m_effectManager()
        , m_respawnMessage(nullptr)
        , m_respawnMessageTimer(sf::Time::Zero)
        , m_scoreText()
        , m_livesText()
        , m_levelText()
        , m_fpsText()
        , m_messageText()
        , m_progressBar()
        , m_scoreFlashTimer(sf::Time::Zero)
        , m_originalScoreColor(sf::Color::White)
        , m_currentLevel(1)
        , m_playerLives(INITIAL_LIVES)
        , m_totalScore(0)
        , m_lastScore(0)
        , m_levelComplete(false)
        , m_levelTransitionTimer(sf::Time::Zero)
        , m_fpsUpdateTime(sf::Time::Zero)
        , m_frameCount(0)
        , m_currentFPS(0.0f)
    {
        auto& window = getGame().getWindow();
        auto& font = getGame().getFonts().get(Fonts::Main);

        // Set player window bounds
        m_player->setWindowBounds(window.getSize());

        // Initialize HUD elements with proper vertical spacing
        float currentY = HUD_MARGIN;
        const float LINE_SPACING = 35.0f;

        // Score text
        m_scoreText.setFont(font);
        m_scoreText.setCharacterSize(HUD_FONT_SIZE);
        m_scoreText.setFillColor(sf::Color::White);
        m_scoreText.setPosition(HUD_MARGIN, currentY);
        m_originalScoreColor = m_scoreText.getFillColor();
        currentY += LINE_SPACING;

        // Lives text
        m_livesText.setFont(font);
        m_livesText.setCharacterSize(HUD_FONT_SIZE);
        m_livesText.setFillColor(sf::Color::White);
        m_livesText.setPosition(HUD_MARGIN, currentY);
        currentY += LINE_SPACING;

        // Level text
        m_levelText.setFont(font);
        m_levelText.setCharacterSize(HUD_FONT_SIZE);
        m_levelText.setFillColor(sf::Color::White);
        m_levelText.setPosition(HUD_MARGIN, currentY);
        currentY += LINE_SPACING;

        // Progress bar - positioned after level text
        m_progressBar.setFont(font);
        m_progressBar.setPosition(HUD_MARGIN, currentY + 10.0f);

        // FPS text (top right)
        m_fpsText.setFont(font);
        m_fpsText.setCharacterSize(HUD_FONT_SIZE);
        m_fpsText.setFillColor(sf::Color::Green);
        m_fpsText.setPosition(window.getSize().x - 100.0f, HUD_MARGIN);

        // Message text (centered)
        m_messageText.setFont(font);
        m_messageText.setCharacterSize(MESSAGE_FONT_SIZE);
        m_messageText.setFillColor(sf::Color::Yellow);
        m_messageText.setOutlineColor(sf::Color::Black);
        m_messageText.setOutlineThickness(2.0f);

        // Set fish spawner level
        m_fishSpawner->setLevel(m_currentLevel);

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
                // TODO: Implement pause state in future stages
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

        // Update visual effects
        m_effectManager.update(deltaTime);

        // Update respawn message
        if (m_respawnMessage && m_respawnMessage->isActive())
        {
            m_respawnMessage->update(deltaTime);
        }

        // Update score flash
        if (m_scoreFlashTimer > sf::Time::Zero)
        {
            m_scoreFlashTimer -= deltaTime;
            float flashIntensity = m_scoreFlashTimer.asSeconds() / SCORE_FLASH_DURATION.asSeconds();
            sf::Color flashColor = sf::Color(
                255,
                static_cast<sf::Uint8>(255 - (255 - m_originalScoreColor.g) * flashIntensity),
                static_cast<sf::Uint8>(255 - (255 - m_originalScoreColor.b) * flashIntensity)
            );
            m_scoreText.setFillColor(flashColor);
        }
        else
        {
            m_scoreText.setFillColor(m_originalScoreColor);
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

        // Update fish spawner
        m_fishSpawner->update(deltaTime, m_currentLevel);

        // Get newly spawned fish
        auto& spawnedFish = m_fishSpawner->getSpawnedFish();
        std::move(spawnedFish.begin(), spawnedFish.end(), std::back_inserter(m_entities));
        m_fishSpawner->clearSpawnedFish();

        // Update player
        m_player->update(deltaTime);

        // Update all entities and their AI
        std::for_each(m_entities.begin(), m_entities.end(),
            [this, deltaTime](const std::unique_ptr<Entity>& entity)
            {
                entity->update(deltaTime);

                // Update fish AI
                Fish* fish = dynamic_cast<Fish*>(entity.get());
                if (fish)
                {
                    fish->updateAI(m_entities, m_player.get(), deltaTime);
                }
            });

        // Remove dead entities using STL erase-remove idiom
        m_entities.erase(
            std::remove_if(m_entities.begin(), m_entities.end(),
                [](const std::unique_ptr<Entity>& entity)
                {
                    return !entity->isAlive();
                }),
            m_entities.end()
        );

        // Check collisions
        checkCollisions();

        // Check for score changes
        if (m_player->getScore() != m_lastScore)
        {
            int pointsGained = m_player->getScore() - m_lastScore;
            if (pointsGained > 0)
            {
                m_scoreFlashTimer = SCORE_FLASH_DURATION;
            }
            m_lastScore = m_player->getScore();
        }

        // Update HUD
        updateHUD();

        // Check for level completion
        if (m_player->getScore() >= LEVEL_COMPLETE_SCORE)
        {
            m_levelComplete = true;
            m_levelTransitionTimer = sf::Time::Zero;

            // Display level complete message
            std::ostringstream messageStream;
            messageStream << "Level " << m_currentLevel << " Complete!";
            m_messageText.setString(messageStream.str());

            // Center the message
            sf::FloatRect bounds = m_messageText.getLocalBounds();
            m_messageText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            m_messageText.setPosition(getGame().getWindow().getSize().x / 2.0f,
                getGame().getWindow().getSize().y / 2.0f);
        }

        return false; // Don't update underlying states
    }

    void PlayState::render()
    {
        auto& window = getGame().getWindow();

        // Draw all entities
        std::for_each(m_entities.begin(), m_entities.end(),
            [&window](const std::unique_ptr<Entity>& entity)
            {
                window.draw(*entity);
            });

        // Draw player
        window.draw(*m_player);

        // Draw visual effects
        m_effectManager.draw(window);

        // Draw HUD
        window.draw(m_scoreText);
        window.draw(m_livesText);
        window.draw(m_levelText);
        window.draw(m_progressBar);
        window.draw(m_fpsText);

        // Draw respawn message if active
        if (m_respawnMessage && m_respawnMessage->isActive())
        {
            m_respawnMessage->draw(window);
        }

        // Draw message if level complete
        if (m_levelComplete)
        {
            window.draw(m_messageText);
        }
    }

    void PlayState::updateHUD()
    {
        // Update score display
        std::ostringstream scoreStream;
        scoreStream << "Score: " << m_player->getScore() << "/" << LEVEL_COMPLETE_SCORE
            << " (Total: " << (m_totalScore + m_player->getScore()) << ")";
        m_scoreText.setString(scoreStream.str());

        // Update lives display
        std::ostringstream livesStream;
        livesStream << "Lives: " << m_playerLives;
        m_livesText.setString(livesStream.str());

        // Update level display
        std::ostringstream levelStream;
        levelStream << "Level: " << m_currentLevel;
        m_levelText.setString(levelStream.str());

        // Update progress bar
        m_progressBar.setStageInfo(m_player->getCurrentStage(), m_player->getScore());
    }

    void PlayState::checkCollisions()
    {
        // Check player collisions with entities
        for (auto& entity : m_entities)
        {
            if (CollisionDetector::checkCircleCollision(*m_player, *entity))
            {
                // Check if it's a fish
                if (entity->getType() == EntityType::SmallFish ||
                    entity->getType() == EntityType::MediumFish ||
                    entity->getType() == EntityType::LargeFish)
                {
                    // Try to eat the fish
                    if (m_player->canEat(*entity))
                    {
                        // Get points from fish
                        const Fish* fish = dynamic_cast<const Fish*>(entity.get());
                        if (fish)
                        {
                            int points = fish->getPointValue();
                            m_player->grow(points);

                            // Create score popup
                            createScorePopup(entity->getPosition(), points);

                            entity->destroy();
                        }
                    }
                    else if (!m_player->isInvulnerable())
                    {
                        // Player was eaten by larger fish
                        handlePlayerDeath();
                    }
                }
            }
        }

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
                    }
                    else if (fish2->canEat(*fish1))
                    {
                        fish1->destroy();
                    }
                }
            }
        }
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
            showRespawnMessage();
        }
    }

    void PlayState::advanceLevel()
    {
        m_currentLevel++;
        m_totalScore += m_player->getScore();
        m_player->resetSize();
        m_levelComplete = false;
        m_lastScore = 0;

        // Clear all entities for new level
        m_entities.clear();
        m_effectManager.clear();

        // Update spawner for new level
        m_fishSpawner->setLevel(m_currentLevel);
    }

    void PlayState::gameOver()
    {
        // Restart the current level
        m_playerLives = INITIAL_LIVES;
        m_player->resetSize();
        m_entities.clear();
        m_effectManager.clear();
        m_lastScore = 0;

        // Display game over message
        auto& font = getGame().getFonts().get(Fonts::Main);
        sf::Text gameOverText("Game Over! Restarting Level " + std::to_string(m_currentLevel),
            font, MESSAGE_FONT_SIZE);
        gameOverText.setFillColor(sf::Color::Red);
        gameOverText.setOutlineColor(sf::Color::Black);
        gameOverText.setOutlineThickness(2.0f);

        sf::FloatRect bounds = gameOverText.getLocalBounds();
        gameOverText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        gameOverText.setPosition(getGame().getWindow().getSize().x / 2.0f,
            getGame().getWindow().getSize().y / 2.0f);

        m_respawnMessage = std::make_unique<FlashingText>(gameOverText, RESPAWN_MESSAGE_DURATION);
    }

    void PlayState::showRespawnMessage()
    {
        auto& font = getGame().getFonts().get(Fonts::Main);
        sf::Text respawnText("Get Ready!", font, MESSAGE_FONT_SIZE);
        respawnText.setFillColor(sf::Color::Yellow);
        respawnText.setOutlineColor(sf::Color::Black);
        respawnText.setOutlineThickness(2.0f);

        sf::FloatRect bounds = respawnText.getLocalBounds();
        respawnText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        respawnText.setPosition(getGame().getWindow().getSize().x / 2.0f,
            getGame().getWindow().getSize().y / 2.0f);

        m_respawnMessage = std::make_unique<FlashingText>(respawnText, RESPAWN_MESSAGE_DURATION);
    }

    void PlayState::createScorePopup(const sf::Vector2f& position, int points)
    {
        auto& font = getGame().getFonts().get(Fonts::Main);
        m_effectManager.createEffect<ScorePopup>(position, points, font);
    }
}