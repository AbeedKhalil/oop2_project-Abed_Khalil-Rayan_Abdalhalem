#include "HUDSystem.h"
#include <sstream>
#include <iomanip>

namespace FishGame
{
    HUDSystem::HUDSystem(const sf::Font& font, const sf::Vector2u& windowSize)
        : m_font(font), m_windowSize(windowSize)
    {
        initText(m_scoreText, Constants::HUD_FONT_SIZE,
            sf::Vector2f(Constants::HUD_MARGIN, Constants::HUD_MARGIN));
        initText(m_livesText, Constants::HUD_FONT_SIZE,
            sf::Vector2f(Constants::HUD_MARGIN, Constants::HUD_MARGIN + Constants::HUD_LINE_SPACING));
        initText(m_levelText, Constants::HUD_FONT_SIZE,
            sf::Vector2f(Constants::HUD_MARGIN, Constants::HUD_MARGIN + Constants::HUD_LINE_SPACING * 2));
        initText(m_chainText, Constants::HUD_SMALL_FONT_SIZE,
            sf::Vector2f(Constants::HUD_MARGIN, Constants::HUD_MARGIN + Constants::HUD_LINE_SPACING * 3));
        initText(m_powerUpText, Constants::HUD_SMALL_FONT_SIZE,
            sf::Vector2f(windowSize.x - Constants::POWERUP_TEXT_X_OFFSET, Constants::HUD_MARGIN + Constants::HUD_LINE_SPACING));
        initText(m_fpsText, Constants::HUD_FONT_SIZE,
            sf::Vector2f(windowSize.x - Constants::FPS_TEXT_X_OFFSET, Constants::HUD_MARGIN));
        initText(m_effectsText, 18,
            sf::Vector2f(Constants::HUD_EFFECTS_TEXT_X,
                windowSize.y - Constants::HUD_EFFECTS_TEXT_Y_OFFSET), sf::Color::Yellow);

        m_messageText.setFont(font);
        m_messageText.setCharacterSize(Constants::MESSAGE_FONT_SIZE);
        m_messageText.setFillColor(Constants::MESSAGE_COLOR);
        m_messageText.setOutlineColor(Constants::MESSAGE_OUTLINE_COLOR);
        m_messageText.setOutlineThickness(Constants::MESSAGE_OUTLINE_THICKNESS);
    }

    void HUDSystem::update(int score, int lives, int level, int chainBonus,
        const std::vector<PowerUpType>& activePowerUps,
        bool frozen, sf::Time freezeTime,
        bool reversed, sf::Time reverseTime,
        bool stunned, sf::Time stunTime,
        float fps)
    {
        std::ostringstream stream;
        stream << "Score: " << score;
        m_scoreText.setString(stream.str());

        stream.str(""); stream.clear();
        stream << "Lives: " << lives;
        m_livesText.setString(stream.str());

        stream.str(""); stream.clear();
        stream << "Level: " << level;
        m_levelText.setString(stream.str());

        if (chainBonus > 0)
        {
            stream.str(""); stream.clear();
            stream << "Chain Bonus: +" << chainBonus;
            m_chainText.setString(stream.str());
        }
        else
        {
            m_chainText.setString("");
        }

        if (!activePowerUps.empty())
        {
            stream.str(""); stream.clear();
            stream << "\nActive Power-Ups:\n";
            for (PowerUpType type : activePowerUps)
            {
                static std::unordered_map<PowerUpType,std::string> names = {
                    {PowerUpType::ScoreDoubler, "2X Score"},
                    {PowerUpType::SpeedBoost, "Speed Boost"},
                    {PowerUpType::Freeze, "Freeze"}
                };
                if (auto it = names.find(type); it != names.end())
                    stream << it->second << "\n";
            }
            m_powerUpText.setString(stream.str());
        }
        else
        {
            m_powerUpText.setString("");
        }

        stream.str(""); stream.clear();
        stream << std::fixed << std::setprecision(1) << "FPS: " << fps;
        m_fpsText.setString(stream.str());

        stream.str(""); stream.clear();
        if (frozen)
            stream << "FREEZE ACTIVE: " << std::fixed << std::setprecision(1)
                << freezeTime.asSeconds() << "s\n";
        if (reversed)
            stream << "CONTROLS REVERSED: " << std::fixed << std::setprecision(1)
                << reverseTime.asSeconds() << "s\n";
        if (stunned)
            stream << "STUNNED: " << std::fixed << std::setprecision(1)
                << stunTime.asSeconds() << "s\n";
        m_effectsText.setString(stream.str());
    }

    void HUDSystem::showMessage(const std::string& message)
    {
        m_messageText.setString(message);
        sf::FloatRect bounds = m_messageText.getLocalBounds();
        m_messageText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        m_messageText.setPosition(m_windowSize.x / 2.f, m_windowSize.y / 2.f);
    }

    void HUDSystem::clearMessage()
    {
        m_messageText.setString("");
    }

    void HUDSystem::initText(sf::Text& text, unsigned int size, const sf::Vector2f& pos, const sf::Color& color)
    {
        text.setFont(m_font);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition(pos);
    }

    void HUDSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_scoreText, states);
        target.draw(m_livesText, states);
        target.draw(m_levelText, states);
        target.draw(m_chainText, states);
        target.draw(m_powerUpText, states);
        target.draw(m_fpsText, states);
        target.draw(m_effectsText, states);
        if (!m_messageText.getString().isEmpty())
            target.draw(m_messageText, states);
    }
}
