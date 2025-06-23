#include "IntroState.h"
#include "Game.h"
#include <algorithm>

namespace FishGame
{
    IntroState::IntroState(Game& game)
        : State(game)
        , m_sprites()
        , m_currentIndex(0)
        , m_elapsed(sf::Time::Zero)
        , m_fadeTime(0.f)
        , m_isFading(false)
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
        m_fadeTime = 0.f;
        m_isFading = false;
        for (auto& sprite : m_sprites)
            sprite.setColor(sf::Color::White);
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
        if (!m_isFading && m_elapsed.asSeconds() >= DISPLAY_DURATION)
        {
            if (m_currentIndex + 1 < m_sprites.size())
            {
                m_isFading = true;
                m_fadeTime = 0.f;
                auto& nextSprite = m_sprites[m_currentIndex + 1];
                sf::Color c = nextSprite.getColor();
                c.a = 0;
                nextSprite.setColor(c);
            }
            else
            {
                deferAction([this]() {
                    requestStackPop();
                    requestStackPush(StateID::Menu);
                    });
            }
        }

        if (m_isFading)
        {
            m_fadeTime += deltaTime.asSeconds();
            float alpha = std::min(m_fadeTime / FADE_DURATION, 1.0f);

            auto setAlpha = [](sf::Sprite& s, float a) {
                sf::Color col = s.getColor();
                col.a = static_cast<sf::Uint8>(255 * a);
                s.setColor(col);
                };

            setAlpha(m_sprites[m_currentIndex], 1.0f - alpha);
            setAlpha(m_sprites[m_currentIndex + 1], alpha);

            if (alpha >= 1.0f)
            {
                m_isFading = false;
                m_elapsed = sf::Time::Zero;
                ++m_currentIndex;
                setAlpha(m_sprites[m_currentIndex], 1.0f);
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
        if (m_isFading && m_currentIndex + 1 < m_sprites.size())
            window.draw(m_sprites[m_currentIndex + 1]);
    }
}
