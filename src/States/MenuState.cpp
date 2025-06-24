#include "MenuState.h"
#include "Game.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include <tuple>

namespace FishGame
{
    MenuState::MenuState(Game& game)
        : State(game)
        , m_titleSprite()
        , m_menuItems()
        , m_selectedOption(MenuOption::NewGame)
        , m_previousOption(MenuOption::NewGame)
        , m_hoveredOption(std::nullopt)
        , m_animationTime(0.0f)
        , m_transitionAlpha(255.0f)
        , m_isTransitioning(false)
        , m_backgroundSprite()
        , m_backgroundFish()
        , m_randomEngine(std::random_device{}())
    {
        initializeBackground();
        initializeMenu();
    }

    void MenuState::initializeMenu()
    {
        auto& window = getGame().getWindow();
        // Setup title sprite
        m_titleSprite.setTexture(
            getGame().getSpriteManager().getTexture(TextureID::GameTitle));
        float scaleFactor = 0.85f;
        m_titleSprite.setScale(scaleFactor, scaleFactor);

        // Center title sprite
        sf::FloatRect titleBounds = m_titleSprite.getLocalBounds();
        m_titleSprite.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        m_titleSprite.setPosition(window.getSize().x / 2.0f, Constants::TITLE_Y_POSITION);

        // Initialize menu items
        const std::array<std::tuple<TextureID, TextureID, MenuAction>, static_cast<size_t>(MenuOption::Count)> menuData = { {
            {TextureID::NewGame, TextureID::NewGameHover, [this]() {
                deferAction([this]() {
                    requestStackPop();
                    requestStackPush(StateID::Play);
                });
            }},
            {TextureID::GameOptions, TextureID::GameOptionsHover, [this]() {
                deferAction([this]() {
                    requestStackPush(StateID::GameOptions);
                });
            }},
            {TextureID::Exit, TextureID::ExitHover, [this]() {
                deferAction([this]() {
                    requestStackClear();
                });
            }}
        } };

        // Setup menu items using STL algorithms
        float yPosition = Constants::MENU_START_Y;

        std::transform(menuData.begin(), menuData.end(), m_menuItems.begin(),
            [this, &window, &yPosition](const auto& data) -> MenuItemType {
                MenuItemType item;
                item.normalTexture = std::get<0>(data);
                item.hoverTexture = std::get<1>(data);
                item.action = std::get<2>(data);

                item.sprite.setTexture(getGame().getSpriteManager().getTexture(item.normalTexture));
                sf::FloatRect bounds = item.sprite.getLocalBounds();
                item.sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                item.sprite.setPosition(window.getSize().x / 2.0f, yPosition);
                item.sprite.setScale(Constants::MENU_BUTTON_SCALE, Constants::MENU_BUTTON_SCALE);

                yPosition += Constants::MENU_ITEM_SPACING;

                return item;
            });

        updateOptionText();
    }

    void MenuState::initializeBackground()
    {
        auto& window = getGame().getWindow();
        m_backgroundSprite.setTexture(
            getGame().getSpriteManager().getTexture(TextureID::Background1));

        sf::Vector2f windowSize(window.getSize());
        sf::Vector2f texSize(m_backgroundSprite.getTexture()->getSize());
        m_backgroundSprite.setScale(windowSize.x / texSize.x,
            windowSize.y / texSize.y);

        std::uniform_real_distribution<float> xDist(0.f, windowSize.x);
        std::uniform_real_distribution<float> yDist(0.f, windowSize.y);
        std::uniform_real_distribution<float> speedDist(20.f, 60.f);
        std::uniform_real_distribution<float> radiusDist(5.f, 15.f);
        std::uniform_int_distribution<int> dirDist(0, 1);

        m_backgroundFish.resize(8);
        for (auto& fish : m_backgroundFish)
        {
            float r = radiusDist(m_randomEngine);
            fish.shape.setRadius(r);
            fish.shape.setOrigin(r, r);
            fish.shape.setFillColor(sf::Color(255, 255, 255, 150));
            fish.shape.setPosition(xDist(m_randomEngine), yDist(m_randomEngine));

            float dir = dirDist(m_randomEngine) ? 1.f : -1.f;
            fish.velocity = sf::Vector2f(dir * speedDist(m_randomEngine), 0.f);
        }
    }

    void MenuState::handleEvent(const sf::Event& event)
    {
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

    void MenuState::handleKeyPress(const sf::Keyboard::Key& key)
    {
        if (m_isTransitioning)
            return;

        switch (key)
        {
        case sf::Keyboard::Up:
        case sf::Keyboard::W:
            m_selectedOption = static_cast<MenuOption>(
                (static_cast<size_t>(m_selectedOption) + static_cast<size_t>(MenuOption::Count) - 1)
                % static_cast<size_t>(MenuOption::Count)
                );
            m_hoveredOption.reset();
            updateOptionText();
            break;

        case sf::Keyboard::Down:
        case sf::Keyboard::S:
            m_selectedOption = static_cast<MenuOption>(
                (static_cast<size_t>(m_selectedOption) + 1) % static_cast<size_t>(MenuOption::Count)
                );
            m_hoveredOption.reset();
            updateOptionText();
            break;

        case sf::Keyboard::Enter:
        case sf::Keyboard::Space:
            selectOption();
            break;

        case sf::Keyboard::Escape:
            deferAction([this]() { requestStackClear(); });
            break;

        default:
            break;
        }
    }

    void MenuState::handleMouseMove(const sf::Vector2f& mousePos)
    {
        if (m_isTransitioning)
            return;

        auto hoveredOption = StateUtils::findItemAt(
            m_menuItems, mousePos,
            [](const auto& item) { return item.sprite.getGlobalBounds(); });

        if (hoveredOption.has_value())
        {
            m_hoveredOption = static_cast<MenuOption>(hoveredOption.value());
            m_selectedOption = static_cast<MenuOption>(hoveredOption.value());
        }
        else
        {
            m_hoveredOption.reset();
        }

        updateOptionText();
    }

    void MenuState::handleMouseClick(const sf::Vector2f& mousePos)
    {
        if (m_isTransitioning)
            return;

        auto clickedOption = StateUtils::findItemAt(
            m_menuItems, mousePos,
            [](const auto& item) { return item.sprite.getGlobalBounds(); });
        if (clickedOption.has_value())
        {
            m_selectedOption = static_cast<MenuOption>(clickedOption.value());
            selectOption();
        }
    }

    bool MenuState::update(sf::Time deltaTime)
    {
        updateBackground(deltaTime);
        updateAnimations(deltaTime);

        // Process any deferred actions
        processDeferredActions();

        return false; // Don't update underlying states
    }

    void MenuState::updateAnimations(sf::Time deltaTime)
    {
        m_animationTime += deltaTime.asSeconds();

        // Update transition fade
        if (m_isTransitioning)
        {
            m_transitionAlpha = std::max(0.0f, m_transitionAlpha - m_fadeSpeed * deltaTime.asSeconds());

            // Apply fade to sprite elements
            auto applyAlphaSprite = [this](sf::Sprite& sprite) {
                sf::Color color = sprite.getColor();
                color.a = static_cast<sf::Uint8>(m_transitionAlpha);
                sprite.setColor(color);
                };

            applyAlphaSprite(m_titleSprite);
            std::for_each(m_menuItems.begin(), m_menuItems.end(),
                [&applyAlphaSprite](auto& item) { applyAlphaSprite(item.sprite); });
        }

        // Animate currently hovered option with pulsing effect
        if (m_hoveredOption.has_value())
        {
            size_t hoveredIndex = static_cast<size_t>(*m_hoveredOption);
            float scale = 1.0f +
                m_pulseAmplitude *
                std::sin(m_animationTime * m_pulseSpeed * 2.0f * Constants::PI);
            StateUtils::applyPulseEffect(
                m_menuItems[hoveredIndex].sprite,
                scale * Constants::MENU_BUTTON_SCALE);
        }
    }

    void MenuState::render()
    {
        auto& window = getGame().getWindow();

        window.draw(m_backgroundSprite);
        for (const auto& fish : m_backgroundFish)
            window.draw(fish.shape);

        window.draw(m_titleSprite);

        // Render all menu items
        std::for_each(m_menuItems.begin(), m_menuItems.end(),
            [&window](const auto& item) {
                window.draw(item.sprite);
            });
    }

    void MenuState::updateBackground(sf::Time deltaTime)
    {
        auto size = getGame().getWindow().getSize();

        for (auto& fish : m_backgroundFish)
        {
            fish.shape.move(fish.velocity * deltaTime.asSeconds());
            sf::Vector2f pos = fish.shape.getPosition();
            float r = fish.shape.getRadius();
            if (fish.velocity.x > 0.f && pos.x - r > static_cast<float>(size.x))
                pos.x = -r;
            else if (fish.velocity.x < 0.f && pos.x + r < 0.f)
                pos.x = static_cast<float>(size.x) + r;
            fish.shape.setPosition(pos);
        }
    }

    void MenuState::updateOptionText()
    {
        // Reset all options to default appearance
        std::for_each(m_menuItems.begin(), m_menuItems.end(),
            [this](auto& item) {
                item.sprite.setTexture(getGame().getSpriteManager().getTexture(item.normalTexture));
                item.sprite.setScale(Constants::MENU_BUTTON_SCALE, Constants::MENU_BUTTON_SCALE);
            });

        if (m_hoveredOption.has_value())
        {
            size_t hoveredIndex = static_cast<size_t>(*m_hoveredOption);
            m_menuItems[hoveredIndex].sprite.setTexture(
                getGame().getSpriteManager().getTexture(m_menuItems[hoveredIndex].hoverTexture));
        }

        // Track previous selection for smooth transitions
        m_previousOption = m_selectedOption;
    }

    void MenuState::selectOption()
    {
        m_isTransitioning = true;
        size_t selectedIndex = static_cast<size_t>(m_selectedOption);
        m_menuItems[selectedIndex].action();
    }

    void MenuState::onActivate()
    {
        // Reset state when menu becomes active
        m_isTransitioning = false;
        m_transitionAlpha = 255.0f;
        m_animationTime = 0.0f;
        m_hoveredOption.reset();
        updateOptionText();
    }
}
