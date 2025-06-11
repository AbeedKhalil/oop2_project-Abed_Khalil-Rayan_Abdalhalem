#include "GameOverState.h"
#include "Game.h"
#include <iomanip>
#include <sstream>
#include <cmath>

namespace FishGame
{
    GameOverState::GameOverState(Game& game)
        : State(game)
        , m_selectedOption(MenuOption::Retry)
        , m_isTransitioning(false)
        , m_transitionAlpha(0.0f)
        , m_animationTime(0.0f)
        , m_fadeInTime(0.0f)
        , m_randomEngine(std::chrono::steady_clock::now().time_since_epoch().count())
    {
        m_particles.reserve(m_maxParticles);
    }

    void GameOverState::onActivate()
    {
        initializeUI();
        initializeStats();
        initializeMenu();
        initializeParticles();
    }

    void GameOverState::handleEvent(const sf::Event& event)
    {
        if (m_isTransitioning) return;

        switch (event.type)
        {
        case sf::Event::KeyPressed:
            handleKeyPress(event.key.code);
            break;
        case sf::Event::MouseMoved:
            handleMouseMove(sf::Vector2f(static_cast<float>(event.mouseMove.x),
                static_cast<float>(event.mouseMove.y)));
            break;
        case sf::Event::MouseButtonPressed:
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                handleMouseClick(sf::Vector2f(static_cast<float>(event.mouseButton.x),
                    static_cast<float>(event.mouseButton.y)));
            }
            break;
        default:
            break;
        }
    }

    bool GameOverState::update(sf::Time deltaTime)
    {
        updateAnimations(deltaTime);
        updateParticles(deltaTime);
        updateMenuSelection(deltaTime);
        updateTransitions(deltaTime);

        // Spawn new particles occasionally
        if (std::uniform_real_distribution<float>(0.0f, 1.0f)(m_randomEngine) < 0.1f)
        {
            spawnParticle(m_randomEngine);
        }

        return false; // Don't block updates to states below
    }

    void GameOverState::render()
    {
        auto& window = getGame().getWindow();

        renderBackground();
        renderParticles();
        renderStats();
        renderMenu();
    }

    void GameOverState::initializeUI()
    {
        auto& window = getGame().getWindow();
        auto& fonts = getGame().getFonts();
        sf::Vector2f windowSize(static_cast<float>(window.getSize().x),
            static_cast<float>(window.getSize().y));

        // Background overlay
        m_backgroundOverlay.setSize(windowSize);
        m_backgroundOverlay.setFillColor(sf::Color(0, 0, 0, 180));

        // Game Over text
        m_gameOverText.setFont(fonts.get(Fonts::Main));
        m_gameOverText.setString("GAME OVER");
        m_gameOverText.setCharacterSize(72);
        m_gameOverText.setFillColor(sf::Color::Red);
        m_gameOverText.setOutlineColor(sf::Color::Black);
        m_gameOverText.setOutlineThickness(3.0f);
        centerText(m_gameOverText, windowSize.y * 0.15f);

        // Title text (for new high score)
        const auto& stats = GameStats::getInstance();
        if (stats.newHighScore)
        {
            m_titleText.setFont(fonts.get(Fonts::Main));
            m_titleText.setString("NEW HIGH SCORE!");
            m_titleText.setCharacterSize(48);
            m_titleText.setFillColor(sf::Color::Yellow);
            m_titleText.setOutlineColor(sf::Color::Black);
            m_titleText.setOutlineThickness(2.0f);
            centerText(m_titleText, windowSize.y * 0.25f);
        }
    }

    void GameOverState::initializeStats()
    {
        const auto& stats = GameStats::getInstance();
        float startY = getGame().getWindow().getSize().y * 0.35f;
        float spacing = 40.0f;

        // Create stat display texts
        createStatText("Final Score", std::to_string(stats.finalScore), startY);
        createStatText("High Score", std::to_string(stats.highScore), startY + spacing);
        createStatText("Fish Eaten", std::to_string(stats.fishEaten), startY + spacing * 2);
        createStatText("Level Reached", std::to_string(stats.levelReached), startY + spacing * 3);

        // Format survival time
        std::stringstream timeStream;
        int minutes = static_cast<int>(stats.survivalTime) / 60;
        int seconds = static_cast<int>(stats.survivalTime) % 60;
        timeStream << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
        createStatText("Survival Time", timeStream.str(), startY + spacing * 4);
    }

    void GameOverState::initializeMenu()
    {
        auto& fonts = getGame().getFonts();
        sf::Vector2f windowSize(static_cast<float>(getGame().getWindow().getSize().x),
            static_cast<float>(getGame().getWindow().getSize().y));
        float menuStartY = windowSize.y * 0.65f;
        float spacing = 60.0f;

        // Define menu items with their actions
        std::array<std::pair<std::string, MenuAction>, static_cast<size_t>(MenuOption::Count)> menuConfig = { {
            {"Retry", [this]() {
                m_isTransitioning = true;
                getGame().clearStates();
                getGame().pushState(StateID::Play);
            }},
            {"Main Menu", [this]() {
                m_isTransitioning = true;
                getGame().clearStates();
                getGame().pushState(StateID::Menu);
            }},
            {"Exit", [this]() {
                m_isTransitioning = true;
                getGame().getWindow().close();
            }}
        } };

        // Initialize menu items using STL algorithms
        size_t index = 0;
        std::transform(menuConfig.begin(), menuConfig.end(), m_menuItems.begin(),
            [&fonts, menuStartY, spacing, windowSize, &index](const auto& config) -> MenuItemType
            {
                MenuItemType item;
                item.text = config.first;
                item.action = config.second;

                // Setup text
                item.textObject.setFont(fonts.get(Fonts::Main));
                item.textObject.setString(item.text);
                item.textObject.setCharacterSize(36);
                item.textObject.setFillColor(sf::Color::White);

                // Calculate position
                float yPos = menuStartY + spacing * index;
                sf::FloatRect bounds = item.textObject.getLocalBounds();
                item.textObject.setOrigin(bounds.left + bounds.width * 0.5f,
                    bounds.top + bounds.height * 0.5f);
                item.textObject.setPosition(windowSize.x * 0.5f, yPos);

                // Setup background
                item.background.setSize(sf::Vector2f(300.0f, 50.0f));
                item.background.setFillColor(sf::Color(0, 0, 0, 0));
                item.background.setOrigin(150.0f, 25.0f);
                item.background.setPosition(windowSize.x * 0.5f, yPos);

                index++;
                return item;
            });

        updateMenuVisuals();
    }

    void GameOverState::initializeParticles()
    {
        // Initialize some particles
        size_t initialParticles = m_maxParticles / 2;
        for (size_t i = 0; i < initialParticles; ++i)
        {
            spawnParticle(m_randomEngine);
        }
    }

    void GameOverState::createStatText(const std::string& label, const std::string& value, float yPos)
    {
        auto& fonts = getGame().getFonts();
        sf::Text statText;

        statText.setFont(fonts.get(Fonts::Main));
        statText.setString(label + ": " + value);
        statText.setCharacterSize(28);
        statText.setFillColor(sf::Color::White);
        statText.setOutlineColor(sf::Color::Black);
        statText.setOutlineThickness(1.0f);

        centerText(statText, yPos);
        m_statTexts.push_back(std::move(statText));
    }

    void GameOverState::updateAnimations(sf::Time deltaTime)
    {
        m_animationTime += deltaTime.asSeconds();
        m_fadeInTime = std::min(m_fadeInTime + deltaTime.asSeconds(), m_fadeInDuration);

        // Pulse effect for game over text
        float pulse = 1.0f + std::sin(m_animationTime * m_pulseSpeed) * m_pulseAmplitude;
        m_gameOverText.setScale(pulse, pulse);

        // Fade in effect
        float fadeAlpha = m_fadeInTime / m_fadeInDuration;
        fadeAlpha = easeInOutQuad(fadeAlpha);

        // Apply fade to all texts using lambda
        auto applyFade = [fadeAlpha](sf::Text& text) {
            sf::Color color = text.getFillColor();
            color.a = static_cast<sf::Uint8>(255 * fadeAlpha);
            text.setFillColor(color);
            };

        applyFade(m_gameOverText);

        const auto& stats = GameStats::getInstance();
        if (stats.newHighScore)
        {
            applyFade(m_titleText);
        }

        std::for_each(m_statTexts.begin(), m_statTexts.end(), applyFade);
        std::for_each(m_menuItems.begin(), m_menuItems.end(),
            [fadeAlpha](auto& item) {
                sf::Color color = item.textObject.getFillColor();
                color.a = static_cast<sf::Uint8>(255 * fadeAlpha);
                item.textObject.setFillColor(color);
            });
    }

    void GameOverState::updateParticles(sf::Time deltaTime)
    {
        float dt = deltaTime.asSeconds();

        // Update existing particles
        auto particleUpdate = [dt](Particle& particle) -> bool {
            particle.lifetime += dt;
            particle.shape.move(particle.velocity * dt);

            // Apply gravity
            particle.velocity.y += 50.0f * dt;

            // Update alpha based on lifetime
            float lifeRatio = particle.lifetime / particle.maxLifetime;
            sf::Color color = particle.shape.getFillColor();
            color.a = static_cast<sf::Uint8>(40 * (1.0f - lifeRatio));
            particle.shape.setFillColor(color);

            return particle.lifetime >= particle.maxLifetime;
            };

        // Remove dead particles using STL algorithm
        m_particles.erase(
            std::remove_if(m_particles.begin(), m_particles.end(), particleUpdate),
            m_particles.end()
        );
    }

    void GameOverState::updateMenuSelection(sf::Time deltaTime)
    {
        if (m_isTransitioning) return;

        // Get mouse position for hover effects
        sf::Vector2i mousePos = sf::Mouse::getPosition(getGame().getWindow());
        sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        // Check hover using STL algorithms
        auto itemIt = std::find_if(m_menuItems.begin(), m_menuItems.end(),
            [mousePosF](const auto& item) {
                return item.background.getGlobalBounds().contains(mousePosF);
            });

        if (itemIt != m_menuItems.end())
        {
            size_t index = std::distance(m_menuItems.begin(), itemIt);
            m_selectedOption = static_cast<MenuOption>(index);
            updateMenuVisuals();
        }
    }

    void GameOverState::updateTransitions(sf::Time deltaTime)
    {
        if (m_isTransitioning)
        {
            m_transitionAlpha += deltaTime.asSeconds() * m_transitionSpeed;
            if (m_transitionAlpha >= 1.0f)
            {
                m_transitionAlpha = 1.0f;
            }

            // Update overlay alpha
            sf::Color overlayColor = m_backgroundOverlay.getFillColor();
            overlayColor.a = static_cast<sf::Uint8>(180 + 75 * m_transitionAlpha);
            m_backgroundOverlay.setFillColor(overlayColor);
        }
    }

    void GameOverState::handleKeyPress(const sf::Keyboard::Key& key)
    {
        switch (key)
        {
        case sf::Keyboard::Up:
        case sf::Keyboard::W:
            navigateMenu(-1);
            break;
        case sf::Keyboard::Down:
        case sf::Keyboard::S:
            navigateMenu(1);
            break;
        case sf::Keyboard::Return:
        case sf::Keyboard::Space:
            selectOption();
            break;
        case sf::Keyboard::Escape:
            m_selectedOption = MenuOption::MainMenu;
            selectOption();
            break;
        default:
            break;
        }
    }

    void GameOverState::handleMouseMove(const sf::Vector2f& mousePos)
    {
        // Hover effect handled in updateMenuSelection
    }

    void GameOverState::handleMouseClick(const sf::Vector2f& mousePos)
    {
        // Check if click is on any menu item using STL
        auto itemIt = std::find_if(m_menuItems.begin(), m_menuItems.end(),
            [mousePos](const auto& item) {
                return item.background.getGlobalBounds().contains(mousePos);
            });

        if (itemIt != m_menuItems.end())
        {
            selectOption();
        }
    }

    void GameOverState::selectOption()
    {
        if (m_isTransitioning) return;

        size_t index = static_cast<size_t>(m_selectedOption);
        if (index < m_menuItems.size())
        {
            m_menuItems[index].action();
        }
    }

    void GameOverState::navigateMenu(int direction)
    {
        int newOption = static_cast<int>(m_selectedOption) + direction;
        int optionCount = static_cast<int>(MenuOption::Count);

        // Wrap around using modulo
        newOption = (newOption % optionCount + optionCount) % optionCount;

        m_selectedOption = static_cast<MenuOption>(newOption);
        updateMenuVisuals();
    }

    void GameOverState::updateMenuVisuals()
    {
        // Use index-based loop with STL transform would be overkill here
        for (size_t i = 0; i < m_menuItems.size(); ++i)
        {
            auto& item = m_menuItems[i];
            bool isSelected = (i == static_cast<size_t>(m_selectedOption));

            // Update text appearance
            if (isSelected)
            {
                item.textObject.setFillColor(sf::Color::Yellow);
                item.textObject.setScale(1.1f, 1.1f);
                item.background.setFillColor(sf::Color(255, 255, 255, 30));
            }
            else
            {
                item.textObject.setFillColor(sf::Color::White);
                item.textObject.setScale(1.0f, 1.0f);
                item.background.setFillColor(sf::Color(0, 0, 0, 0));
            }
        }
    }

    void GameOverState::renderBackground()
    {
        auto& window = getGame().getWindow();
        window.draw(m_backgroundOverlay);
    }

    void GameOverState::renderStats()
    {
        auto& window = getGame().getWindow();
        const auto& stats = GameStats::getInstance();

        window.draw(m_gameOverText);

        if (stats.newHighScore)
        {
            window.draw(m_titleText);
        }

        // Render all stat texts using STL
        std::for_each(m_statTexts.begin(), m_statTexts.end(),
            [&window](const auto& text) {
                window.draw(text);
            });
    }

    void GameOverState::renderMenu()
    {
        auto& window = getGame().getWindow();

        // Render menu items using STL
        std::for_each(m_menuItems.begin(), m_menuItems.end(),
            [&window](const auto& item) {
                window.draw(item.background);
                window.draw(item.textObject);
            });
    }

    void GameOverState::renderParticles()
    {
        auto& window = getGame().getWindow();

        // Render all particles using STL
        std::for_each(m_particles.begin(), m_particles.end(),
            [&window](const auto& particle) {
                window.draw(particle.shape);
            });
    }

    void GameOverState::centerText(sf::Text& text, float yPosition)
    {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
        text.setPosition(static_cast<float>(getGame().getWindow().getSize().x) * 0.5f, yPosition);
    }

    sf::Color GameOverState::interpolateColor(const sf::Color& start, const sf::Color& end, float t)
    {
        // Ensure t is clamped between 0 and 1
        t = std::clamp(t, 0.0f, 1.0f);

        return sf::Color(
            static_cast<sf::Uint8>(start.r + (end.r - start.r) * t),
            static_cast<sf::Uint8>(start.g + (end.g - start.g) * t),
            static_cast<sf::Uint8>(start.b + (end.b - start.b) * t),
            static_cast<sf::Uint8>(start.a + (end.a - start.a) * t)
        );
    }

    float GameOverState::easeInOutQuad(float t)
    {
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    }
}