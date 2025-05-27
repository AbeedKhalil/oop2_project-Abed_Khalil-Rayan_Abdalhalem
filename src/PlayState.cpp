// PlayState.cpp
#include "PlayState.h"
#include "Game.h"
#include "CollisionDetector.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <random>

namespace FishGame
{
    // Static member initialization
    const sf::Time PlayState::m_spawnInterval = sf::seconds(2.0f);

    PlayState::PlayState(Game& game)
        : State(game)
        , m_player(std::make_unique<Player>())
        , m_entities()
        , m_scoreText()
        , m_livesText()
        , m_levelText()
        , m_fpsText()
        , m_currentLevel(1)
        , m_playerLives(3)
        , m_playerScore(0)
        , m_fpsUpdateTime(sf::Time::Zero)
        , m_frameCount(0)
        , m_currentFPS(0.0f)
        , m_spawnTimer(sf::Time::Zero)
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

        // Initialize entities for testing
        initializeEntities();
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

        // Update spawn timer for test fish
        m_spawnTimer += deltaTime;
        if (m_spawnTimer >= m_spawnInterval)
        {
            spawnTestFish();
            m_spawnTimer = sf::Time::Zero;
        }

        // Update player
        m_player->update(deltaTime);

        // Update all entities
        std::for_each(m_entities.begin(), m_entities.end(),
            [deltaTime](const std::unique_ptr<Entity>& entity)
            {
                entity->update(deltaTime);
            });

        // Remove dead entities using erase-remove idiom
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

        // Check for level progression
        if (m_playerScore >= 100)
        {
            m_currentLevel++;
            m_playerScore = 0;
            m_player->resetSize();
            // TODO: Implement level transition in future stages
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
    }

    void PlayState::initializeEntities()
    {
        // For Stage 1, we'll spawn a few test fish
        // These will be replaced with proper spawning system in Stage 3
        spawnTestFish();
    }

    void PlayState::updateHUD()
    {
        std::ostringstream scoreStream;
        scoreStream << "Score: " << m_playerScore;
        m_scoreText.setString(scoreStream.str());

        std::ostringstream livesStream;
        livesStream << "Lives: " << m_playerLives;
        m_livesText.setString(livesStream.str());

        std::ostringstream levelStream;
        levelStream << "Level: " << m_currentLevel;
        m_levelText.setString(levelStream.str());
    }

    void PlayState::checkCollisions()
    {
        // Check player collisions with entities
        for (auto& entity : m_entities)
        {
            if (CollisionDetector::checkCircleCollision(*m_player, *entity))
            {
                // For Stage 1, just destroy the entity and add score
                entity->destroy();
                m_player->grow();
                m_playerScore += 10;
            }
        }
    }

    void PlayState::spawnTestFish()
    {
        // Create a simple test entity for Stage 1
        // This will be replaced with proper fish types in Stage 2
        class TestFish : public Entity
        {
        public:
            TestFish(float x, float y, float radius, const sf::Color& color)
                : Entity()
                , m_shape(radius)
            {
                m_position = sf::Vector2f(x, y);
                m_radius = radius;
                m_shape.setFillColor(color);
                m_shape.setOrigin(radius, radius);
                m_shape.setPosition(m_position);

                // Simple horizontal movement
                m_velocity = sf::Vector2f(100.0f, 0.0f);
            }

            void update(sf::Time deltaTime) override
            {
                updateMovement(deltaTime);
                m_shape.setPosition(m_position);

                // Destroy if off screen
                if (m_position.x > 2000.0f || m_position.x < -100.0f)
                {
                    destroy();
                }
            }

            sf::FloatRect getBounds() const override
            {
                return sf::FloatRect(m_position.x - m_radius, m_position.y - m_radius,
                    m_radius * 2.0f, m_radius * 2.0f);
            }

            EntityType getType() const override
            {
                return EntityType::SmallFish;
            }

        protected:
            void draw(sf::RenderTarget& target, sf::RenderStates states) const override
            {
                target.draw(m_shape, states);
            }

        private:
            sf::CircleShape m_shape;
        };

        // Random number generation using STL
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> yDist(100.0f, 980.0f);
        std::uniform_real_distribution<float> sizeDist(15.0f, 30.0f);

        float radius = sizeDist(gen);
        sf::Color color = (radius < 20.0f) ? sf::Color::Green : sf::Color::Blue;

        auto fish = std::make_unique<TestFish>(-50.0f, yDist(gen), radius, color);
        m_entities.push_back(std::move(fish));
    }
}