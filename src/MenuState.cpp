#include "MenuState.h"
#include "Game.h"
#include <cmath>
#include <algorithm>

namespace FishGame
{
    MenuState::MenuState(Game& game)
        : State(game)
        , m_titleText()
        , m_options()
        , m_selectedOption(MenuOption::Play)
        , m_animationTime(0.0f)
    {
        auto& window = getGame().getWindow();
        auto& font = getGame().getFonts().get(Fonts::Main);

        // Setup title
        m_titleText.setFont(font);
        m_titleText.setString("FEEDING FRENZY");
        m_titleText.setCharacterSize(72);
        m_titleText.setFillColor(sf::Color::Yellow);
        m_titleText.setOutlineColor(sf::Color::Black);
        m_titleText.setOutlineThickness(3.0f);

        // Center title
        sf::FloatRect titleBounds = m_titleText.getLocalBounds();
        m_titleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        m_titleText.setPosition(window.getSize().x / 2.0f, 200.0f);

        // Setup menu options
        const std::array<std::string, static_cast<size_t>(MenuOption::Count)> optionStrings = {
            "Play Game",
            "Options",
            "Exit"
        };

        float yPosition = 400.0f;
        const float ySpacing = 80.0f;

        for (size_t i = 0; i < m_options.size(); ++i)
        {
            m_options[i].setFont(font);
            m_options[i].setString(optionStrings[i]);
            m_options[i].setCharacterSize(48);
            m_options[i].setFillColor(sf::Color::White);

            // Center each option
            sf::FloatRect optionBounds = m_options[i].getLocalBounds();
            m_options[i].setOrigin(optionBounds.width / 2.0f, optionBounds.height / 2.0f);
            m_options[i].setPosition(window.getSize().x / 2.0f, yPosition);

            yPosition += ySpacing;
        }

        updateOptionText();
    }

    void MenuState::handleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
            case sf::Keyboard::Up:
            case sf::Keyboard::W:
                if (m_selectedOption > MenuOption::Play)
                {
                    m_selectedOption = static_cast<MenuOption>(static_cast<int>(m_selectedOption) - 1);
                    updateOptionText();
                }
                break;

            case sf::Keyboard::Down:
            case sf::Keyboard::S:
                if (m_selectedOption < MenuOption::Exit)
                {
                    m_selectedOption = static_cast<MenuOption>(static_cast<int>(m_selectedOption) + 1);
                    updateOptionText();
                }
                break;

            case sf::Keyboard::Enter:
            case sf::Keyboard::Space:
                selectOption();
                break;

            case sf::Keyboard::Escape:
                requestStackClear();
                break;

            default:
                break;
            }
        }
        else if (event.type == sf::Event::MouseMoved)
        {
            // Check mouse hover over options
            sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x),
                static_cast<float>(event.mouseMove.y));

            for (size_t i = 0; i < m_options.size(); ++i)
            {
                if (m_options[i].getGlobalBounds().contains(mousePos))
                {
                    m_selectedOption = static_cast<MenuOption>(i);
                    updateOptionText();
                    break;
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left)
        {
            // Check mouse click on options
            sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x),
                static_cast<float>(event.mouseButton.y));

            for (size_t i = 0; i < m_options.size(); ++i)
            {
                if (m_options[i].getGlobalBounds().contains(mousePos))
                {
                    m_selectedOption = static_cast<MenuOption>(i);
                    selectOption();
                    break;
                }
            }
        }
    }

    bool MenuState::update(sf::Time deltaTime)
    {
        m_animationTime += deltaTime.asSeconds();

        // Animate selected option with pulsing effect
        size_t selectedIndex = static_cast<size_t>(m_selectedOption);
        float scale = 1.0f + 0.1f * std::sin(m_animationTime * m_pulseSpeed * 2.0f * 3.14159f);
        m_options[selectedIndex].setScale(scale, scale);

        return false; // Don't update underlying states
    }

    void MenuState::render()
    {
        auto& window = getGame().getWindow();

        window.draw(m_titleText);

        std::for_each(m_options.begin(), m_options.end(),
            [&window](const sf::Text& option) { window.draw(option); });
    }

    void MenuState::updateOptionText()
    {
        // Reset all options to default appearance
        std::for_each(m_options.begin(), m_options.end(),
            [](sf::Text& option)
            {
                option.setFillColor(sf::Color::White);
                option.setScale(1.0f, 1.0f);
            });

        // Highlight selected option
        size_t selectedIndex = static_cast<size_t>(m_selectedOption);
        m_options[selectedIndex].setFillColor(sf::Color::Yellow);
    }

    void MenuState::selectOption()
    {
        switch (m_selectedOption)
        {
        case MenuOption::Play:
            requestStackPop();
            requestStackPush(StateID::Play);
            break;

        case MenuOption::Options:
            // TODO: Implement options state in future stages
            break;

        case MenuOption::Exit:
            requestStackClear();
            break;

        default:
            break;
        }
    }
}