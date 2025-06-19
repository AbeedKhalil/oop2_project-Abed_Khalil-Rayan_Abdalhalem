#include "IntroState.h"
#include "Game.h"

namespace FishGame
{
    IntroState::IntroState(Game& game)
        : State(game)
        , m_sprites()
        , m_currentIndex(0)
        , m_elapsed(sf::Time::Zero)
    {
    }

    void IntroState::onActivate()
    {
        auto& window = getGame().getWindow();
        auto& manager = getGame().getSpriteManager();
        m_sprites[0].setTexture(manager.getTexture(TextureID::Intro1));
        m_sprites[1].setTexture(manager.getTexture(TextureID::Intro2));

        for (auto& sprite : m_sprites)
        {
            auto size = sprite.getTexture()->getSize();
            sprite.setScale(
                static_cast<float>(window.getSize().x) / size.x,
                static_cast<float>(window.getSize().y) / size.y);
        }

        m_currentIndex = 0;
        m_elapsed = sf::Time::Zero;
    }

    void IntroState::handleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed ||
            event.type == sf::Event::MouseButtonPressed)
        {
            deferAction([this]() {
                requestStackPop();
                requestStackPush(StateID::Menu);
                });
        }
    }

    bool IntroState::update(sf::Time deltaTime)
    {
        m_elapsed += deltaTime;
        if (m_elapsed.asSeconds() >= DISPLAY_DURATION)
        {
            m_elapsed -= sf::seconds(DISPLAY_DURATION);
            ++m_currentIndex;
            if (m_currentIndex >= m_sprites.size())
            {
                deferAction([this]() {
                    requestStackPop();
                    requestStackPush(StateID::Menu);
                    });
            }
        }

        processDeferredActions();
        return false;
    }

    void IntroState::render()
    {
        auto& window = getGame().getWindow();
        if (m_currentIndex < m_sprites.size())
            window.draw(m_sprites[m_currentIndex]);
    }
}