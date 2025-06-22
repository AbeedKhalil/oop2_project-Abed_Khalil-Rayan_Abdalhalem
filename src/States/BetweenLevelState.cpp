#include "BetweenLevelState.h"
#include "Game.h"

namespace FishGame
{
    namespace
    {
        std::vector<std::string> g_upcomingEntities;
    }

    void setBetweenLevelEntities(std::vector<std::string> entities)
    {
        g_upcomingEntities = std::move(entities);
    }

    static std::vector<std::string> takeEntities()
    {
        std::vector<std::string> tmp;
        tmp.swap(g_upcomingEntities);
        return tmp;
    }

    BetweenLevelState::BetweenLevelState(Game& game)
        : State(game)
        , m_entities(takeEntities())
        , m_headerText()
        , m_continueText()
        , m_entityTexts()
        , m_background()
    {
    }

    void BetweenLevelState::onActivate()
    {
        auto& window = getGame().getWindow();
        auto& font = getGame().getFonts().get(Fonts::Main);

        m_background.setSize(sf::Vector2f(window.getSize()));
        m_background.setFillColor(Constants::OVERLAY_COLOR);

        m_headerText.setFont(font);
        m_headerText.setString("Upcoming Creatures");
        m_headerText.setCharacterSize(48);
        m_headerText.setFillColor(sf::Color::White);
        sf::FloatRect hb = m_headerText.getLocalBounds();
        m_headerText.setOrigin(hb.width / 2.f, hb.height / 2.f);
        m_headerText.setPosition(window.getSize().x / 2.f, 150.f);

        m_continueText.setFont(font);
        m_continueText.setString("Press Enter to start");
        m_continueText.setCharacterSize(32);
        m_continueText.setFillColor(sf::Color::White);
        sf::FloatRect cb = m_continueText.getLocalBounds();
        m_continueText.setOrigin(cb.width / 2.f, cb.height / 2.f);
        m_continueText.setPosition(window.getSize().x / 2.f, window.getSize().y - 150.f);

        float y = 250.f;
        for (const auto& name : m_entities)
        {
            sf::Text text;
            text.setFont(font);
            text.setString(name);
            text.setCharacterSize(32);
            text.setFillColor(sf::Color::Yellow);
            sf::FloatRect tb = text.getLocalBounds();
            text.setOrigin(tb.width / 2.f, tb.height / 2.f);
            text.setPosition(window.getSize().x / 2.f, y);
            y += 40.f;
            m_entityTexts.push_back(text);
        }
    }

    void BetweenLevelState::handleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
        {
            deferAction([this]() { requestStackPop(); });
        }
    }

    bool BetweenLevelState::update(sf::Time deltaTime)
    {
        processDeferredActions();
        (void)deltaTime;
        return false;
    }

    void BetweenLevelState::render()
    {
        auto& window = getGame().getWindow();
        window.draw(m_background);
        window.draw(m_headerText);
        for (const auto& t : m_entityTexts)
        {
            window.draw(t);
        }
        window.draw(m_continueText);
    }
}

