#include "VisualEffects.h"
#include "GameConstants.h"
#include <cmath>
#include <sstream>

namespace FishGame
{
    VisualEffect::VisualEffect(sf::Time duration)
        : m_timeRemaining(duration)
        , m_totalDuration(duration)
    {
    }

    FlashingText::FlashingText(const sf::Text& text, sf::Time duration, float flashSpeed)
        : VisualEffect(duration)
        , m_text(text)
        , m_flashSpeed(flashSpeed)
        , m_currentAlpha(255.0f)
    {
    }

    void FlashingText::update(sf::Time deltaTime)
    {
        m_timeRemaining -= deltaTime;

        // Calculate alpha based on sine wave
        float timeRatio = 1.0f - (m_timeRemaining.asSeconds() / m_totalDuration.asSeconds());
        m_currentAlpha = 128.0f + 127.0f * std::sin(timeRatio * m_flashSpeed * 2.0f * 3.14159f);

        sf::Color color = m_text.getFillColor();
        color.a = static_cast<sf::Uint8>(m_currentAlpha);
        m_text.setFillColor(color);

        sf::Color outlineColor = m_text.getOutlineColor();
        outlineColor.a = static_cast<sf::Uint8>(m_currentAlpha);
        m_text.setOutlineColor(outlineColor);
    }

    void FlashingText::draw(sf::RenderTarget& target) const
    {
        target.draw(m_text);
    }

    ScorePopup::ScorePopup(const sf::Vector2f& position, int points, const sf::Font& font)
        : VisualEffect(Constants::SCORE_FLASH_DURATION)
        , m_text()
        , m_velocity(0.0f, -50.0f)
        , m_fadeSpeed(2.0f)
    {
        std::ostringstream stream;
        stream << "+" << points;

        m_text.setFont(font);
        m_text.setString(stream.str());
        m_text.setCharacterSize(32);
        m_text.setFillColor(sf::Color::Yellow);
        m_text.setOutlineColor(sf::Color::Black);
        m_text.setOutlineThickness(2.0f);

        sf::FloatRect bounds = m_text.getLocalBounds();
        m_text.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        m_text.setPosition(position);
    }

    void ScorePopup::update(sf::Time deltaTime)
    {
        m_timeRemaining -= deltaTime;

        // Move upward
        sf::Vector2f position = m_text.getPosition();
        position += m_velocity * deltaTime.asSeconds();
        m_text.setPosition(position);

        // Fade out
        float alpha = 255.0f * (m_timeRemaining.asSeconds() / m_totalDuration.asSeconds());
        sf::Color color = m_text.getFillColor();
        color.a = static_cast<sf::Uint8>(alpha);
        m_text.setFillColor(color);

        sf::Color outlineColor = m_text.getOutlineColor();
        outlineColor.a = static_cast<sf::Uint8>(alpha);
        m_text.setOutlineColor(outlineColor);
    }

    void ScorePopup::draw(sf::RenderTarget& target) const
    {
        target.draw(m_text);
    }
}