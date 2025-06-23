#include "GameOptionsState.h"
#include "Game.h"

namespace FishGame
{
    GameOptionsState::GameOptionsState(Game& game)
        : State(game)
        , m_titleText()
        , m_instructionText()
        , m_background()
    {
    }

    void GameOptionsState::onActivate()
    {
        auto& window = getGame().getWindow();
        auto& font = getGame().getFonts().get(Fonts::Main);

        m_background.setSize(sf::Vector2f(window.getSize()));
        m_background.setFillColor(Constants::OVERLAY_COLOR);

        m_titleText.setFont(font);
        m_titleText.setString("OPTIONS");
        m_titleText.setCharacterSize(72);
        m_titleText.setFillColor(sf::Color::White);
        auto bounds = m_titleText.getLocalBounds();
        m_titleText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        m_titleText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 40.f);

        m_instructionText.setFont(font);
        m_instructionText.setString("Press Esc to return");
        m_instructionText.setCharacterSize(36);
        m_instructionText.setFillColor(sf::Color::White);
        bounds = m_instructionText.getLocalBounds();
        m_instructionText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        m_instructionText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f + 40.f);
    }

    void GameOptionsState::handleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
        {
            deferAction([this]() { requestStackPop(); });
        }
    }

    bool GameOptionsState::update(sf::Time)
    {
        processDeferredActions();
        return false;
    }

    void GameOptionsState::render()
    {
        auto& window = getGame().getWindow();
        window.draw(m_background);
        window.draw(m_titleText);
        window.draw(m_instructionText);
    }
}
