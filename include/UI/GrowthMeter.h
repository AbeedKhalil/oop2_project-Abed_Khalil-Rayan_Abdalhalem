#pragma once

#include <SFML/Graphics.hpp>
#include <functional>

namespace FishGame
{
    class GrowthMeter : public sf::Drawable
    {
    public:
        explicit GrowthMeter(const sf::Font& font);
        ~GrowthMeter() = default;

        // Delete copy operations
        GrowthMeter(const GrowthMeter&) = delete;
        GrowthMeter& operator=(const GrowthMeter&) = delete;

        // Allow move operations
        GrowthMeter(GrowthMeter&&) = default;
        GrowthMeter& operator=(GrowthMeter&&) = default;

        // Core functionality
        void setPoints(int points);
        void update(sf::Time deltaTime);
        void reset();
        void setPosition(float x, float y);
        void setStage(int stage);

        // Callbacks
        void setOnStageComplete(std::function<void()> callback) { m_onStageComplete = callback; }

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updateVisuals();

    private:
        // Visual components
        sf::RectangleShape m_background;
        sf::RectangleShape m_fillBar;
        sf::RectangleShape m_border;
        sf::Text m_stageText;
        sf::Text m_progressText;

        // Meter properties
        float m_currentProgress;
        float m_maxProgress;
        int m_currentStage;
        sf::Vector2f m_position;

        // Points tracking
        int m_points;

        // Animation
        float m_targetProgress;
        static constexpr float m_fillSpeed = 200.0f;
        float m_glowIntensity;

        // Visual properties
        static constexpr float m_width = 300.0f;
        static constexpr float m_height = 30.0f;
        static constexpr float m_borderThickness = 2.0f;

        // Callback
        std::function<void()> m_onStageComplete;
    };
}