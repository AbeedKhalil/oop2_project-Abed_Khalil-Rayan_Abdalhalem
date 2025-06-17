#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <functional>
#include <numeric>

namespace FishGame
{
    // Score event types for tracking
    enum class ScoreEventType
    {
        FishEaten,
        BonusCollected,
        TailBite,
        PowerUpCollected,
        LevelComplete
    };

    // Floating score text that appears when points are earned
    class FloatingScore : public sf::Drawable
    {
    public:
        FloatingScore(const sf::Font& font, int points, int multiplier, sf::Vector2f position);

        void update(sf::Time deltaTime);
        bool isExpired() const { return m_lifetime >= m_maxLifetime; }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        sf::Text m_text;
        sf::Vector2f m_velocity;
        sf::Time m_lifetime;
        float m_alpha;

        static const sf::Time m_maxLifetime;
        static constexpr float m_floatSpeed = -100.0f;
        static constexpr float m_fadeSpeed = 170.0f;
    };

    class ScoreSystem
    {
    public:
        ScoreSystem(const sf::Font& font);
        ~ScoreSystem() = default;

        // Delete copy operations
        ScoreSystem(const ScoreSystem&) = delete;
        ScoreSystem& operator=(const ScoreSystem&) = delete;

        // Allow move operations
        ScoreSystem(ScoreSystem&&) = default;
        ScoreSystem& operator=(ScoreSystem&&) = default;

        // Score calculation
        int calculateScore(ScoreEventType type, int basePoints, int frenzyMultiplier, float powerUpMultiplier);
        void addScore(ScoreEventType type, int basePoints, sf::Vector2f position, int frenzyMultiplier, float powerUpMultiplier);

        // Chain system
        void registerHit(); // Successful fish eaten
        void registerMiss(); // Missed attempt or damage taken
        void updateChain(sf::Time deltaTime);
        int getChainBonus() const { return m_currentChain; }

        // Tail-bite mechanic
        void registerTailBite(sf::Vector2f position, int frenzyMultiplier, float powerUpMultiplier);

        // End-of-level bonuses
        int calculateTimeBonus(sf::Time completionTime, sf::Time targetTime);
        int calculateGrowthBonus(bool reachedMaxSize);
        int calculateUntouchableBonus(bool tookNoDamage);

        // Update and rendering
        void update(sf::Time deltaTime);
        void drawFloatingScores(sf::RenderTarget& target) const;

        // Score tracking
        int getCurrentScore() const { return m_currentScore; }
        int getTotalScore() const { return m_totalScore; }
        void setCurrentScore(int score) { m_currentScore = score; }
        void addToTotalScore(int score) { m_totalScore += score; }
        void reset();

    private:
        void createFloatingScore(int points, int multiplier, sf::Vector2f position);

    private:
        const sf::Font& m_font;

        // Score tracking
        int m_currentScore;
        int m_totalScore;

        // Chain system
        int m_currentChain;
        static constexpr int m_maxChain = 10;

        // Visual elements
        std::vector<std::unique_ptr<FloatingScore>> m_floatingScores;

        // Bonus values
        static constexpr int m_tailBiteBonus = 75;
        static constexpr int m_timeBonusMax = 500;
        static constexpr int m_growthBonus = 1000;
        static constexpr int m_untouchableBonus = 2000;
    };

    // Template function for calculating scores with different multipliers
    template<typename... Multipliers>
    int calculateTotalScore(int baseScore, Multipliers... multipliers)
    {
        return baseScore * (... * multipliers);
    }
}