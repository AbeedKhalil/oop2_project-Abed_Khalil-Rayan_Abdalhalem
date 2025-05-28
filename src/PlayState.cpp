// PlayState.cpp
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
    const sf::Time PlayState::m_levelTransitionDuration = sf::seconds(3.0f);

    PlayState::PlayState(Game& game)
        : State(game)
        , m_player(std::make_unique<Player>())
        , m_fishSpawner(std::make_unique<FishSpawner>(getGame().getWindow().getSize()))
        , m_entities()
        , m_scoreText()
        , m_livesText()
        , m_levelText()
        , m_fpsText()
        , m_messageText()
        , m_currentLevel(1)
        , m_playerLives(3)
        , m_totalScore(0)
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

        // Update HUD
        updateHUD();

        // Check for level completion
        if (m_player->getScore() >= 100) // Score requirement is now 100 points
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

        // Draw HUD
        window.draw(m_scoreText);
        window.draw(m_livesText);
        window.draw(m_levelText);
        window.draw(m_fpsText);

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
        scoreStream << "Score: " << m_player->getScore() << "/100 (Total: " << m_totalScore << ")";
        m_scoreText.setString(scoreStream.str());

        // Update lives display
        std::ostringstream livesStream;
        livesStream << "Lives: " << m_playerLives;
        m_livesText.setString(livesStream.str());

        // Update level display
        std::ostringstream levelStream;
        levelStream << "Level: " << m_currentLevel << " | Stage: " << m_player->getCurrentStage();
        m_levelText.setString(levelStream.str());
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
                            m_player->grow(fish->getPointValue());
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
        }
    }

    void PlayState::advanceLevel()
    {
        m_currentLevel++;
        m_totalScore += m_player->getScore();
        m_player->resetSize();
        m_levelComplete = false;

        // Clear all entities for new level
        m_entities.clear();

        // Update spawner for new level
        m_fishSpawner->setLevel(m_currentLevel);
    }

    void PlayState::gameOver()
    {
        // Restart the current level instead of going to menu
        m_playerLives = 3; // Reset lives
        m_player->resetSize(); // Reset to stage 1
        m_entities.clear(); // Clear all fish

        // Display game over message temporarily
        m_messageText.setString("Game Over! Restarting Level " + std::to_string(m_currentLevel));
        sf::FloatRect bounds = m_messageText.getLocalBounds();
        m_messageText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        m_messageText.setPosition(getGame().getWindow().getSize().x / 2.0f,
            getGame().getWindow().getSize().y / 2.0f);

        // Note: In future stages, you might want to add a delay here
        // to show the game over message before restarting
    }
}