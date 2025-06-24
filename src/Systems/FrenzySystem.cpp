#include "FrenzySystem.h"
#include "GameConstants.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace FishGame
{
    FrenzySystem::FrenzySystem(const sf::Font& font)
        : m_eatHistory()
        , m_currentLevel(FrenzyLevel::None)
        , m_frenzyTimer(sf::Time::Zero)
        , m_frenzyText()
        , m_multiplierText()
        , m_timerText()
        , m_timerBar(sf::Vector2f(m_timerBarWidth, m_timerBarHeight))
        , m_timerBackground(sf::Vector2f(m_timerBarWidth, m_timerBarHeight))
        , m_position(0.0f, 0.0f)
        , m_textScale(1.0f)
        , m_textRotation(0.0f)
        , m_currentColor(sf::Color::White)
        , m_animationTimer(sf::Time::Zero)
    {
        // Setup frenzy text
        m_frenzyText.setFont(font);
        m_frenzyText.setCharacterSize(36);
        m_frenzyText.setFillColor(sf::Color::Yellow);
        m_frenzyText.setOutlineColor(sf::Color::Black);
        m_frenzyText.setOutlineThickness(2.0f);

        // Setup multiplier text
        m_multiplierText.setFont(font);
        m_multiplierText.setCharacterSize(Constants::HUD_FONT_SIZE);
        m_multiplierText.setFillColor(sf::Color::White);

        // Setup timer text
        m_timerText.setFont(font);
        m_timerText.setCharacterSize(18);
        m_timerText.setFillColor(sf::Color::White);

        // Setup timer bar
        m_timerBar.setFillColor(sf::Color::Yellow);
        m_timerBackground.setFillColor(sf::Color(50, 50, 50, 150));
        m_timerBackground.setOutlineColor(sf::Color::White);
        m_timerBackground.setOutlineThickness(1.0f);
    }

    void FrenzySystem::registerFishEaten()
    {
        // Add new eat event
        m_eatHistory.push_back({ sf::Time::Zero });

        // Reset frenzy timer if active
        if (m_currentLevel != FrenzyLevel::None)
        {
            m_frenzyTimer = sf::seconds(m_frenzyMaintainTime);
        }

        updateFrenzyState();
    }

    void FrenzySystem::update(sf::Time deltaTime)
    {
        // Update eat history timestamps
        std::for_each(m_eatHistory.begin(), m_eatHistory.end(),
            [deltaTime](EatEvent& event) { event.timestamp += deltaTime; });

        // Remove old events based on current state
        float timeWindow = (m_currentLevel == FrenzyLevel::None) ? m_frenzyActivationTime : m_superFrenzyActivationTime;

        m_eatHistory.erase(
            std::remove_if(m_eatHistory.begin(), m_eatHistory.end(),
                [timeWindow](const EatEvent& event) { return event.timestamp.asSeconds() > timeWindow; }),
            m_eatHistory.end()
        );

        // Update frenzy timer
        if (m_currentLevel != FrenzyLevel::None)
        {
            m_frenzyTimer -= deltaTime;
            if (m_frenzyTimer <= sf::Time::Zero)
            {
                setFrenzyLevel(FrenzyLevel::None);
            }
        }

        // Update visuals
        updateVisuals(deltaTime);
    }

    void FrenzySystem::reset()
    {
        m_eatHistory.clear();
        m_frenzyTimer = sf::Time::Zero;
        setFrenzyLevel(FrenzyLevel::None);
        m_textScale = 1.0f;
        m_textRotation = 0.0f;
    }

    void FrenzySystem::forceFrenzy()
    {
        setFrenzyLevel(FrenzyLevel::Frenzy);
        m_frenzyTimer = sf::seconds(m_frenzyMaintainTime);

        // Clear history to prevent immediate super frenzy
        m_eatHistory.clear();
    }

    void FrenzySystem::setPosition(float x, float y)
    {
        m_position = sf::Vector2f(x, y);

        // Update positions
        m_frenzyText.setPosition(m_position);
        m_multiplierText.setPosition(m_position.x, m_position.y + 40.0f);
        m_timerText.setPosition(m_position.x, m_position.y + 65.0f);
        m_timerBackground.setPosition(m_position.x, m_position.y + 90.0f);
        m_timerBar.setPosition(m_position.x, m_position.y + 90.0f);
    }

    void FrenzySystem::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (m_currentLevel != FrenzyLevel::None)
        {
            // Apply animation transforms
            sf::Transform transform;
            transform.translate(m_frenzyText.getPosition());
            transform.rotate(m_textRotation);
            transform.scale(m_textScale, m_textScale);
            transform.translate(-m_frenzyText.getPosition());

            states.transform *= transform;

            target.draw(m_frenzyText, states);
            target.draw(m_multiplierText);
            target.draw(m_timerText);
            target.draw(m_timerBackground);
            target.draw(m_timerBar);
        }
    }

    void FrenzySystem::updateFrenzyState()
    {
        size_t recentEats = m_eatHistory.size();

        if (m_currentLevel == FrenzyLevel::None && recentEats >= m_requiredFishCount)
        {
            // Check if all fish were eaten within activation window
            float timeSpan = m_eatHistory.back().timestamp.asSeconds();
            if (timeSpan <= m_frenzyActivationTime)
            {
                setFrenzyLevel(FrenzyLevel::Frenzy);
                m_frenzyTimer = sf::seconds(m_frenzyMaintainTime);
            }
        }
        else if (m_currentLevel == FrenzyLevel::Frenzy && recentEats >= m_requiredFishCount)
        {
            // Check for super frenzy upgrade
            auto recentStart = std::find_if(m_eatHistory.begin(), m_eatHistory.end(),
                [](const EatEvent& event) { return event.timestamp.asSeconds() <= 2.5f; });

            std::size_t recentCount =
                static_cast<std::size_t>(std::distance(recentStart, m_eatHistory.end()));
            if (recentCount >= m_requiredFishCount)
            {
                setFrenzyLevel(FrenzyLevel::SuperFrenzy);
                m_frenzyTimer = sf::seconds(m_frenzyMaintainTime);
            }
        }
    }

    void FrenzySystem::setFrenzyLevel(FrenzyLevel level)
    {
        if (m_currentLevel != level)
        {
            m_currentLevel = level;
            m_animationTimer = sf::Time::Zero;

            // Update visuals based on level
            switch (level)
            {
            case FrenzyLevel::None:
                break;

            case FrenzyLevel::Frenzy:
                m_frenzyText.setString("FRENZY!");
                m_multiplierText.setString("2X Score Multiplier");
                m_currentColor = sf::Color::Yellow;
                m_textScale = 1.5f;
                break;

            case FrenzyLevel::SuperFrenzy:
                m_frenzyText.setString("SUPER FRENZY!");
                m_multiplierText.setString("4X Score Multiplier");
                m_currentColor = sf::Color::Magenta;
                m_textScale = 2.0f;
                break;
            }

            // Center text
            sf::FloatRect bounds = m_frenzyText.getLocalBounds();
            m_frenzyText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);

            bounds = m_multiplierText.getLocalBounds();
            m_multiplierText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        }
    }

    void FrenzySystem::updateVisuals(sf::Time deltaTime)
    {
        if (m_currentLevel != FrenzyLevel::None)
        {
            m_animationTimer += deltaTime;

            // Animate text
            m_textScale = 1.0f + 0.1f * std::sin(m_animationTimer.asSeconds() * 5.0f);
            m_textRotation = 5.0f * std::sin(m_animationTimer.asSeconds() * 3.0f);

            // Update timer bar
            float timerPercentage = m_frenzyTimer.asSeconds() / m_frenzyMaintainTime;
            m_timerBar.setSize(sf::Vector2f(m_timerBarWidth * timerPercentage, m_timerBarHeight));

            // Update timer text
            std::ostringstream timerStream;
            timerStream << "Time: " << std::fixed << std::setprecision(1)
                << m_frenzyTimer.asSeconds() << "s";
            m_timerText.setString(timerStream.str());

            // Flash color
            float flash = std::abs(std::sin(m_animationTimer.asSeconds() * 10.0f));
            sf::Color flashColor = m_currentColor;
            flashColor.r = static_cast<sf::Uint8>(flashColor.r + (255 - flashColor.r) * flash * 0.3f);
            flashColor.g = static_cast<sf::Uint8>(flashColor.g + (255 - flashColor.g) * flash * 0.3f);
            flashColor.b = static_cast<sf::Uint8>(flashColor.b + (255 - flashColor.b) * flash * 0.3f);

            m_frenzyText.setFillColor(flashColor);
            m_timerBar.setFillColor(flashColor);
        }
    }
}
