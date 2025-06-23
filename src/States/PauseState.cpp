#include "PauseState.h"
#include "Game.h"

namespace FishGame
{
    PauseState::PauseState(Game& game)
        : State(game)
        , m_pauseText()
        , m_instructionText()
        , m_background()
    {
    }

    void PauseState::onActivate()
    {
        auto& window = getGame().getWindow();
        auto& font = getGame().getFonts().get(Fonts::Main);

        m_background.setSize(sf::Vector2f(window.getSize()));
        m_background.setFillColor(Constants::OVERLAY_COLOR);

        m_pauseText.setFont(font);
        m_pauseText.setString("PAUSED");
        m_pauseText.setCharacterSize(72);
        m_pauseText.setFillColor(sf::Color::White);
        sf::FloatRect bounds = m_pauseText.getLocalBounds();
        m_pauseText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        m_pauseText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 40.f);

        m_instructionText.setFont(font);
        m_instructionText.setString("Press P or Esc to resume");
        m_instructionText.setCharacterSize(36);
        m_instructionText.setFillColor(sf::Color::White);
        bounds = m_instructionText.getLocalBounds();
        m_instructionText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        m_instructionText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f + 40.f);
    }

    void PauseState::handleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::P)
            {
                deferAction([this]() { requestStackPop(); });
            }
        }
    }

    bool PauseState::update(sf::Time)
    {
        processDeferredActions();
        return false;
    }

    void PauseState::render()
    {
        auto& window = getGame().getWindow();
        window.draw(m_background);
        window.draw(m_pauseText);
        window.draw(m_instructionText);
    }
}
