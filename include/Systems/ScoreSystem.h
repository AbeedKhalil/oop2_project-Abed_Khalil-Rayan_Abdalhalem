#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <cmath>
#include <unordered_map>
#include "SpriteManager.h"

namespace FishGame
{
    // Score event types for tracking
    enum class ScoreEventType
    {
        FishEaten,
        BonusCollected,
        TailBite
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
        int getChainBonus() const { return m_currentChain; }

        // Tail-bite mechanic
        void registerTailBite(sf::Vector2f position, int frenzyMultiplier, float powerUpMultiplier);

        // Update and rendering
        void update(sf::Time deltaTime);
        void drawFloatingScores(sf::RenderTarget& target) const;

        // Score tracking
        int getCurrentScore() const { return m_currentScore; }
        void setCurrentScore(int score) { m_currentScore = score; }
        void recordFish(TextureID id);
        const std::unordered_map<TextureID,int>& getFishCounts() const { return m_fishCounts; }
        void reset();

    private:
        void createFloatingScore(int points, int multiplier, sf::Vector2f position);

    private:
        const sf::Font& m_font;

        // Score tracking
        int m_currentScore;

        // Chain system
        int m_currentChain;
        static constexpr int m_maxChain = 10;

        // Visual elements
        std::vector<std::unique_ptr<FloatingScore>> m_floatingScores;

        // Fish counts
        std::unordered_map<TextureID,int> m_fishCounts;

        // Bonus values
        static constexpr int m_tailBiteBonus = 75;
    };

    // Template function for calculating scores with different multipliers
    template<typename... Multipliers>
    int calculateTotalScore(int baseScore, Multipliers... multipliers)
    {
        float total = static_cast<float>(baseScore);
        ((total *= static_cast<float>(multipliers)), ...);
        return static_cast<int>(std::round(total));
    }
}
