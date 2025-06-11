#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <deque>
#include <functional>

namespace FishGame
{
    enum class FrenzyLevel
    {
        None = 1,
        Frenzy = 2,
        SuperFrenzy = 4
    };

    class FrenzySystem : public sf::Drawable
    {
    public:
        explicit FrenzySystem(const sf::Font& font);
        ~FrenzySystem() = default;

        // Delete copy operations
        FrenzySystem(const FrenzySystem&) = delete;
        FrenzySystem& operator=(const FrenzySystem&) = delete;

        // Allow move operations
        FrenzySystem(FrenzySystem&&) = default;
        FrenzySystem& operator=(FrenzySystem&&) = default;

        // Core functionality
        void registerFishEaten();
        void update(sf::Time deltaTime);
        void reset();
        void forceFrenzy(); // For Frenzy Starter power-up

        // Getters
        FrenzyLevel getCurrentLevel() const { return m_currentLevel; }
        int getMultiplier() const { return static_cast<int>(m_currentLevel); }
        bool isActive() const { return m_currentLevel != FrenzyLevel::None; }
        float getRemainingTime() const { return m_frenzyTimer.asSeconds(); }

        // Setters
        void setPosition(float x, float y);

        // Callbacks
        void setOnFrenzyStart(std::function<void(FrenzyLevel)> callback) { m_onFrenzyStart = callback; }
        void setOnFrenzyEnd(std::function<void()> callback) { m_onFrenzyEnd = callback; }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        struct EatEvent
        {
            sf::Time timestamp = sf::Time::Zero;
        };

        void updateFrenzyState();
        void setFrenzyLevel(FrenzyLevel level);
        void updateVisuals(sf::Time deltaTime);

    private:
        // Frenzy tracking
        std::deque<EatEvent> m_eatHistory;
        FrenzyLevel m_currentLevel;
        sf::Time m_frenzyTimer;

        // Visual components
        sf::Text m_frenzyText;
        sf::Text m_multiplierText;
        sf::Text m_timerText;
        sf::RectangleShape m_timerBar;
        sf::RectangleShape m_timerBackground;
        sf::Vector2f m_position;

        // Animation
        float m_textScale;
        float m_textRotation;
        sf::Color m_currentColor;

        // Timing constants
        static constexpr float m_frenzyActivationTime = 2.0f;     // 4 fish in 2 seconds
        static constexpr float m_superFrenzyActivationTime = 2.5f; // 4 more in 2.5 seconds
        static constexpr float m_frenzyMaintainTime = 2.5f;       // Maintain timer
        static constexpr int m_requiredFishCount = 4;

        // Visual constants
        static constexpr float m_timerBarWidth = 200.0f;
        static constexpr float m_timerBarHeight = 10.0f;

        // Callbacks
        std::function<void(FrenzyLevel)> m_onFrenzyStart;
        std::function<void()> m_onFrenzyEnd;
    };
}