#include "MenuState.h"
#include "Game.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace FishGame
{
    MenuState::MenuState(Game& game)
        : State(game)
        , m_titleText()
        , m_menuItems()
        , m_selectedOption(MenuOption::Play)
        , m_previousOption(MenuOption::Play)
        , m_animationTime(0.0f)
        , m_transitionAlpha(255.0f)
        , m_isTransitioning(false)
    {
        initializeMenu();
    }

    void MenuState::initializeMenu()
    {
        auto& window = getGame().getWindow();
        auto& font = getGame().getFonts().get(Fonts::Main);

        // Setup title using constants
        m_titleText.setFont(font);
        m_titleText.setString(Constants::GAME_TITLE);
        m_titleText.setCharacterSize(Constants::TITLE_FONT_SIZE);
        m_titleText.setFillColor(Constants::TITLE_COLOR);
        m_titleText.setOutlineColor(Constants::TITLE_OUTLINE_COLOR);
        m_titleText.setOutlineThickness(Constants::TITLE_OUTLINE_THICKNESS);

        // Center title
        sf::FloatRect titleBounds = m_titleText.getLocalBounds();
        m_titleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        m_titleText.setPosition(window.getSize().x / 2.0f, Constants::TITLE_Y_POSITION);

        // Initialize menu items
        const std::array<std::pair<std::string, MenuAction>, static_cast<size_t>(MenuOption::Count)> menuData = { {
            {"Play Game", [this]() {
                deferAction([this]() {
                    requestStackPop();
                    requestStackPush(StateID::Play);
                });
            }},
            {"Settings", [this]() {
                deferAction([this]() {
                    requestStackPush(StateID::Settings);
                });
            }},
            {"Credits", [this]() {
                deferAction([this]() {
                    requestStackPush(StateID::Credits);
                });
            }},
            {"Exit", [this]() {
                deferAction([this]() {
                    requestStackClear();
                });
            }}
        } };

        // Setup menu items using STL algorithms
        float yPosition = Constants::MENU_START_Y;

        std::transform(menuData.begin(), menuData.end(), m_menuItems.begin(),
            [&font, &window, &yPosition](const auto& data) -> MenuItemType {
                MenuItemType item;
                item.text = data.first;
                item.action = data.second;

                item.textObject.setFont(font);
                item.textObject.setString(item.text);
                item.textObject.setCharacterSize(Constants::MENU_FONT_SIZE);
                item.textObject.setFillColor(Constants::MENU_NORMAL_COLOR);

                // Center each option
                sf::FloatRect bounds = item.textObject.getLocalBounds();
                item.textObject.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                item.textObject.setPosition(window.getSize().x / 2.0f, yPosition);

                yPosition += Constants::MENU_ITEM_SPACING;

                return item;
            });

        updateOptionText();
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
            updateOptionText();
            break;

        case sf::Keyboard::Down:
        case sf::Keyboard::S:
            m_selectedOption = static_cast<MenuOption>(
                (static_cast<size_t>(m_selectedOption) + 1) % static_cast<size_t>(MenuOption::Count)
                );
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

        auto hoveredOption = findOptionAt(m_menuItems, mousePos);
        if (hoveredOption.has_value())
        {
            m_selectedOption = static_cast<MenuOption>(hoveredOption.value());
            updateOptionText();
        }
    }

    void MenuState::handleMouseClick(const sf::Vector2f& mousePos)
    {
        if (m_isTransitioning)
            return;

        auto clickedOption = findOptionAt(m_menuItems, mousePos);
        if (clickedOption.has_value())
        {
            m_selectedOption = static_cast<MenuOption>(clickedOption.value());
            selectOption();
        }
    }

    bool MenuState::update(sf::Time deltaTime)
    {
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

            // Apply fade to all text elements
            auto applyAlpha = [this](sf::Text& text) {
                sf::Color color = text.getFillColor();
                color.a = static_cast<sf::Uint8>(m_transitionAlpha);
                text.setFillColor(color);
                };

            applyAlpha(m_titleText);
            std::for_each(m_menuItems.begin(), m_menuItems.end(),
                [&applyAlpha](auto& item) { applyAlpha(item.textObject); });
        }

        // Animate selected option with pulsing effect
        size_t selectedIndex = static_cast<size_t>(m_selectedOption);
        float scale = 1.0f + m_pulseAmplitude * std::sin(m_animationTime * m_pulseSpeed * 2.0f * Constants::PI);
        applyPulseEffect(m_menuItems[selectedIndex].textObject, scale);
    }

    void MenuState::applyPulseEffect(sf::Text& text, float scale)
    {
        text.setScale(scale, scale);
    }

    void MenuState::render()
    {
        auto& window = getGame().getWindow();

        window.draw(m_titleText);

        // Render all menu items
        std::for_each(m_menuItems.begin(), m_menuItems.end(),
            [&window](const auto& item) {
                window.draw(item.textObject);
            });
    }

    void MenuState::updateOptionText()
    {
        // Reset all options to default appearance
        std::for_each(m_menuItems.begin(), m_menuItems.end(),
            [](auto& item) {
                item.textObject.setFillColor(Constants::MENU_NORMAL_COLOR);
                item.textObject.setScale(1.0f, 1.0f);
            });

        // Highlight selected option
        size_t selectedIndex = static_cast<size_t>(m_selectedOption);
        m_menuItems[selectedIndex].textObject.setFillColor(Constants::MENU_SELECTED_COLOR);

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
        updateOptionText();
    }
}