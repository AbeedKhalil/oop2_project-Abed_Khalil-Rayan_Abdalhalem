#include "ScoreSystem.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace FishGame
{
    // Static member initialization
    const sf::Time FloatingScore::m_maxLifetime = sf::seconds(1.5f);

    // FloatingScore implementation
    FloatingScore::FloatingScore(const sf::Font& font, int points, int multiplier, sf::Vector2f position)
        : m_text()
        , m_velocity(0.0f, m_floatSpeed)
        , m_lifetime(sf::Time::Zero)
        , m_alpha(255.0f)
    {
        m_text.setFont(font);
        m_text.setPosition(position);

        // Format score text with multiplier
        std::ostringstream scoreStream;
        scoreStream << "+" << points;
        if (multiplier > 1)
        {
            scoreStream << " x" << multiplier;
        }
        m_text.setString(scoreStream.str());

        // Set appearance based on score magnitude
        if (points >= 500)
        {
            m_text.setCharacterSize(32);
            m_text.setFillColor(sf::Color::Magenta);
            m_text.setOutlineThickness(2.0f);
        }
        else if (points >= 100)
        {
            m_text.setCharacterSize(28);
            m_text.setFillColor(sf::Color::Yellow);
            m_text.setOutlineThickness(1.5f);
        }
        else
        {
            m_text.setCharacterSize(24);
            m_text.setFillColor(sf::Color::White);
            m_text.setOutlineThickness(1.0f);
        }

        m_text.setOutlineColor(sf::Color::Black);

        // Center text origin
        sf::FloatRect bounds = m_text.getLocalBounds();
        m_text.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
    }

    void FloatingScore::update(sf::Time deltaTime)
    {
        m_lifetime += deltaTime;

        // Update position
        m_text.move(m_velocity * deltaTime.asSeconds());

        // Update alpha fade
        float fadeProgress = m_lifetime.asSeconds() / m_maxLifetime.asSeconds();
        m_alpha = std::max(0.0f, 255.0f * (1.0f - fadeProgress));

        sf::Color color = m_text.getFillColor();
        color.a = static_cast<sf::Uint8>(m_alpha);
        m_text.setFillColor(color);

        sf::Color outlineColor = m_text.getOutlineColor();
        outlineColor.a = static_cast<sf::Uint8>(m_alpha);
        m_text.setOutlineColor(outlineColor);

        // Scale effect
        float scale = 1.0f + fadeProgress * 0.5f;
        m_text.setScale(scale, scale);
    }

    void FloatingScore::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_text, states);
    }

    // ScoreSystem implementation
    ScoreSystem::ScoreSystem(const sf::Font& font)
        : m_font(font)
        , m_currentScore(0)
        , m_totalScore(0)
        , m_currentChain(0)
        , m_floatingScores()
        , m_recentEvents()
        , m_timeSinceStart(sf::Time::Zero)
    {
        m_floatingScores.reserve(20);
        m_recentEvents.resize(0);
    }

    int ScoreSystem::calculateScore(ScoreEventType type, int basePoints, int frenzyMultiplier, float powerUpMultiplier)
    {
        // Calculate total score using template function
        int totalPoints = calculateTotalScore(basePoints, frenzyMultiplier, powerUpMultiplier);

        // Add chain bonus for fish eaten
        if (type == ScoreEventType::FishEaten && m_currentChain > 0)
        {
            totalPoints += m_currentChain;
        }

        return totalPoints;
    }

    void ScoreSystem::addScore(ScoreEventType type, int basePoints, sf::Vector2f position,
        int frenzyMultiplier, float powerUpMultiplier)
    {
        int totalPoints = calculateScore(type, basePoints, frenzyMultiplier, powerUpMultiplier);

        // Update score
        m_currentScore += totalPoints;

        // Create floating score
        int displayMultiplier = static_cast<int>(frenzyMultiplier * powerUpMultiplier);
        createFloatingScore(totalPoints, displayMultiplier, position);

        // Record event
        m_recentEvents.push_back({
            type,
            basePoints,
            totalPoints,
            position,
            m_timeSinceStart
            });

        // Maintain event history size
        if (m_recentEvents.size() > m_maxRecentEvents)
        {
            m_recentEvents.pop_front();
        }
    }

    void ScoreSystem::registerHit()
    {
        m_currentChain = std::min(m_currentChain + 1, m_maxChain);
    }

    void ScoreSystem::registerMiss()
    {
        m_currentChain = 0;
    }

    void ScoreSystem::updateChain(sf::Time deltaTime)
    {
        // Chain doesn't decay over time in this implementation
        // Could add decay logic here if needed
    }

    void ScoreSystem::registerTailBite(sf::Vector2f position, int frenzyMultiplier, float powerUpMultiplier)
    {
        addScore(ScoreEventType::TailBite, m_tailBiteBonus, position, frenzyMultiplier, powerUpMultiplier);
    }

    int ScoreSystem::calculateTimeBonus(sf::Time completionTime, sf::Time targetTime)
    {
        if (completionTime <= targetTime)
        {
            float ratio = 1.0f - (completionTime.asSeconds() / targetTime.asSeconds());
            return static_cast<int>(m_timeBonusMax * ratio);
        }
        return 0;
    }

    int ScoreSystem::calculateGrowthBonus(bool reachedMaxSize)
    {
        return reachedMaxSize ? m_growthBonus : 0;
    }

    int ScoreSystem::calculateUntouchableBonus(bool tookNoDamage)
    {
        return tookNoDamage ? m_untouchableBonus : 0;
    }

    void ScoreSystem::update(sf::Time deltaTime)
    {
        m_timeSinceStart += deltaTime;

        // Update floating scores
        std::for_each(m_floatingScores.begin(), m_floatingScores.end(),
            [deltaTime](std::unique_ptr<FloatingScore>& score) {
                score->update(deltaTime);
            });

        // Remove expired floating scores using erase-remove idiom
        m_floatingScores.erase(
            std::remove_if(m_floatingScores.begin(), m_floatingScores.end(),
                [](const std::unique_ptr<FloatingScore>& score) {
                    return score->isExpired();
                }),
            m_floatingScores.end()
        );

        updateRecentEvents();
    }

    void ScoreSystem::drawFloatingScores(sf::RenderTarget& target) const
    {
        std::for_each(m_floatingScores.begin(), m_floatingScores.end(),
            [&target](const std::unique_ptr<FloatingScore>& score) {
                target.draw(*score);
            });
    }

    void ScoreSystem::reset()
    {
        m_currentScore = 0;
        m_currentChain = 0;
        m_floatingScores.clear();
        m_recentEvents.clear();
        m_timeSinceStart = sf::Time::Zero;
    }

    float ScoreSystem::getAverageScorePerSecond() const
    {
        if (m_timeSinceStart <= sf::Time::Zero)
            return 0.0f;

        // Calculate total score from recent events
        int recentScore = std::accumulate(m_recentEvents.begin(), m_recentEvents.end(), 0,
            [](int sum, const ScoreEvent& event) {
                return sum + event.totalPoints;
            });

        // Get time span of recent events
        if (!m_recentEvents.empty())
        {
            sf::Time timeSpan = m_timeSinceStart - m_recentEvents.front().timestamp;
            if (timeSpan > sf::Time::Zero)
            {
                return static_cast<float>(recentScore) / timeSpan.asSeconds();
            }
        }

        return static_cast<float>(m_currentScore) / m_timeSinceStart.asSeconds();
    }

    void ScoreSystem::createFloatingScore(int points, int multiplier, sf::Vector2f position)
    {
        auto floatingScore = std::make_unique<FloatingScore>(m_font, points, multiplier, position);
        m_floatingScores.push_back(std::move(floatingScore));
    }

    void ScoreSystem::updateRecentEvents()
    {
        // Remove events older than 30 seconds
        const sf::Time maxEventAge = sf::seconds(30.0f);

        m_recentEvents.erase(
            std::remove_if(m_recentEvents.begin(), m_recentEvents.end(),
                [this, maxEventAge](const ScoreEvent& event) {
                    return (m_timeSinceStart - event.timestamp) > maxEventAge;
                }),
            m_recentEvents.end()
        );
    }
}