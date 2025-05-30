// GrowthMeter.h
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <functional>

namespace FishGame
{
    class GrowthMeter : public sf::Drawable
    {
    public:
        GrowthMeter(const sf::Font& font);
        ~GrowthMeter() = default;

        // Delete copy operations
        GrowthMeter(const GrowthMeter&) = delete;
        GrowthMeter& operator=(const GrowthMeter&) = delete;

        // Allow move operations
        GrowthMeter(GrowthMeter&&) = default;
        GrowthMeter& operator=(GrowthMeter&&) = default;

        // Core functionality
        void addProgress(float points);
        void update(sf::Time deltaTime);
        void reset();
        void setPosition(float x, float y);
        void setStage(int stage);

        // Getters
        float getCurrentProgress() const { return m_currentProgress; }
        float getMaxProgress() const { return m_maxProgress; }
        bool isStageComplete() const { return m_currentProgress >= m_maxProgress; }
        int getCurrentStage() const { return m_currentStage; }

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

        // Animation
        float m_targetProgress;
        static constexpr float m_fillSpeed = 200.0f;
        float m_glowIntensity;

        // Visual properties
        static constexpr float m_width = 300.0f;
        static constexpr float m_height = 30.0f;
        static constexpr float m_borderThickness = 2.0f;

        // Stage progression
        // Stage progression - 4 stages total
        static constexpr float m_stage1Progress = 100.0f;  // Points to reach stage 2
        static constexpr float m_stage2Progress = 150.0f;  // Points to reach stage 3
        static constexpr float m_stage3Progress = 200.0f;  // Points to reach stage 4
        static constexpr float m_stage4Progress = 250.0f;  // Maximum (for display only)

        // Callback
        std::function<void()> m_onStageComplete;
    };
}