#include "BetweenLevelState.h"
#include "Game.h"
#include "Levels/LevelTable.h"
#include "SpriteManager.h"
#include <optional>

namespace FishGame
{
    namespace
    {
        LevelDef g_upcomingDef;

        std::optional<TextureID> textureFromName(const std::string& name)
        {
            if (name == "SmallFish") return TextureID::SmallFish;
            if (name == "MediumFish") return TextureID::MediumFish;
            if (name == "LargeFish") return TextureID::LargeFish;
            if (name == "Fish") return TextureID::SmallFish;
            if (name == "Barracuda") return TextureID::Barracuda;
            if (name == "Pufferfish") return TextureID::Pufferfish;
            if (name == "Angelfish") return TextureID::Angelfish;
            if (name == "PoisonFish") return TextureID::PoisonFish;
            if (name == "Bomb") return TextureID::Bomb;
            if (name == "Jellyfish") return TextureID::Jellyfish;
            if (name == "Oyster") return TextureID::PearlOysterClosed;
            if (name == "Life") return TextureID::PowerUpExtraLife;
            if (name == "Speed") return TextureID::PowerUpSpeedBoost;
            if (name == "Add-Time") return TextureID::PowerUpAddTime;
            // Power-ups without textures are skipped
            return std::nullopt;
        }
    }

    void setUpcomingLevelDef(LevelDef def)
    {
        g_upcomingDef = std::move(def);
    }

    LevelDef takeUpcomingLevelDef()
    {
        LevelDef tmp;
        tmp = std::move(g_upcomingDef);
        g_upcomingDef = LevelDef{};
        return tmp;
    }

    BetweenLevelState::BetweenLevelState(Game& game, LevelDef upcoming)
        : State(game)
        , m_def(std::move(upcoming))
        , m_headerText()
        , m_continueText()
        , m_items()
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

        auto& spriteMgr = getGame().getSpriteManager();

        float y = 250.f;
        for (const auto& info : m_def.enemies)
        {
            DisplayItem item;
            item.text.setFont(font);
            item.text.setCharacterSize(32);
            item.text.setFillColor(sf::Color::Yellow);
            item.text.setString("x" + std::to_string(info.count));

            if (auto texId = textureFromName(info.type))
            {
                item.sprite.setTexture(spriteMgr.getTexture(*texId));
                item.sprite.setScale(0.5f, 0.5f);
                sf::FloatRect sb = item.sprite.getLocalBounds();
                item.sprite.setOrigin(sb.width / 2.f, sb.height / 2.f);
                item.hasSprite = true;
            }
            else
            {
                item.text.setString(info.type + " x" + std::to_string(info.count));
            }

            item.sprite.setPosition(window.getSize().x / 2.f - 40.f, y);
            item.text.setPosition(window.getSize().x / 2.f + 40.f, y - 10.f);
            y += 60.f;
            m_items.push_back(std::move(item));
        }

        for (const auto& name : m_def.powerUps)
        {
            DisplayItem item;
            item.text.setFont(font);
            item.text.setCharacterSize(32);
            item.text.setFillColor(sf::Color::Cyan);
            item.text.setString(name);

            if (auto texId = textureFromName(name))
            {
                item.sprite.setTexture(spriteMgr.getTexture(*texId));
                item.sprite.setScale(0.5f, 0.5f);
                sf::FloatRect sb = item.sprite.getLocalBounds();
                item.sprite.setOrigin(sb.width / 2.f, sb.height / 2.f);
                item.hasSprite = true;
                item.text.setPosition(window.getSize().x / 2.f + 40.f, y - 10.f);
                item.sprite.setPosition(window.getSize().x / 2.f - 40.f, y);
            }
            else
            {
                item.text.setPosition(window.getSize().x / 2.f, y);
            }

            y += 60.f;
            m_items.push_back(std::move(item));
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
        for (const auto& item : m_items)
        {
            if (item.hasSprite)
                window.draw(item.sprite);
            window.draw(item.text);
        }
        window.draw(m_continueText);
    }
}

