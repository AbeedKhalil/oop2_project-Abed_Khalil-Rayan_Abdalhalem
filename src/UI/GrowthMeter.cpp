#include "GrowthMeter.h"
#include "GameConstants.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace FishGame
{
    GrowthMeter::GrowthMeter(const sf::Font& font)
        : m_background(sf::Vector2f(m_width, m_height))
        , m_fillBar(sf::Vector2f(0.0f, m_height - m_borderThickness * 2))
        , m_border(sf::Vector2f(m_width, m_height))
        , m_stageText()
        , m_progressText()
        , m_currentProgress(0.0f)
        , m_maxProgress(m_stage1Progress)
        , m_currentStage(1)
        , m_position(0.0f, 0.0f)
        , m_targetProgress(0.0f)
        , m_glowIntensity(0.0f)
        , m_onStageComplete(nullptr)
    {
        // Setup background
        m_background.setFillColor(sf::Color(30, 30, 30, 200));
        m_background.setOutlineThickness(0);

        // Setup fill bar
        m_fillBar.setFillColor(sf::Color(0, 255, 100));
        m_fillBar.setPosition(m_borderThickness, m_borderThickness);

        // Setup border
        m_border.setFillColor(sf::Color::Transparent);
        m_border.setOutlineColor(sf::Color::White);
        m_border.setOutlineThickness(m_borderThickness);

        // Setup stage text
        m_stageText.setFont(font);
        m_stageText.setCharacterSize(20);
        m_stageText.setFillColor(sf::Color::White);
        m_stageText.setString("Stage 1");

        // Setup progress text
        m_progressText.setFont(font);
        m_progressText.setCharacterSize(16);
        m_progressText.setFillColor(sf::Color::White);

        updateVisuals();
    }

    void GrowthMeter::addProgress(float points)
    {
        m_targetProgress = std::min(m_targetProgress + points, m_maxProgress);

        // Check if stage complete
        if (m_targetProgress >= m_maxProgress && m_onStageComplete && m_currentStage < 4)
        {
            m_onStageComplete();

            // Move to next stage's progress tracking
            if (m_currentStage < 3)  // Only reset progress for stages 1-2
            {
                m_currentProgress = 0.0f;
                m_targetProgress = 0.0f;
            }
        }
    }

    void GrowthMeter::update(sf::Time deltaTime)
    {
        // Smooth progress animation
        if (m_currentProgress < m_targetProgress)
        {
            float increment = m_fillSpeed * deltaTime.asSeconds();
            m_currentProgress = std::min(m_currentProgress + increment, m_targetProgress);
            updateVisuals();
        }

        // Check for stage advancement
        if (m_currentProgress >= m_maxProgress && m_currentStage < Constants::MAX_STAGES)
        {
            if (m_onStageComplete)
            {
                m_onStageComplete();
            }

            // Advance to next stage
            if (m_currentStage < Constants::MAX_STAGES - 1)  // Only advance if not at final stage
            {
                m_currentStage++;
                setStage(m_currentStage);
                m_currentProgress = 0.0f;
                m_targetProgress = 0.0f;
            }
            else if (m_currentStage == Constants::MAX_STAGES - 1)
            {
                // Special handling for stage 2 to 3 transition
                m_currentStage = Constants::MAX_STAGES;
                setStage(Constants::MAX_STAGES);
                // Keep progress at max for stage 3
            }
        }

        // Glow effect when near completion
        if (m_currentProgress / m_maxProgress > 0.8f && m_currentStage < 4)
        {
            m_glowIntensity = std::abs(std::sin(deltaTime.asSeconds() * 3.0f)) * 0.5f + 0.5f;

            sf::Color fillColor = m_fillBar.getFillColor();
            fillColor.r = static_cast<sf::Uint8>(fillColor.r + (255 - fillColor.r) * m_glowIntensity * 0.3f);
            fillColor.g = static_cast<sf::Uint8>(fillColor.g + (255 - fillColor.g) * m_glowIntensity * 0.3f);
            m_fillBar.setFillColor(fillColor);
        }
    }

    void GrowthMeter::reset()
    {
        m_currentProgress = 0.0f;
        m_targetProgress = 0.0f;
        m_glowIntensity = 0.0f;
        updateVisuals();
    }

    void GrowthMeter::setPosition(float x, float y)
    {
        m_position = sf::Vector2f(x, y);
        m_background.setPosition(m_position);
        m_fillBar.setPosition(m_position.x + m_borderThickness, m_position.y + m_borderThickness);
        m_border.setPosition(m_position);

        // Position text elements
        m_stageText.setPosition(m_position.x + 5.0f, m_position.y - 25.0f);

        sf::FloatRect progressBounds = m_progressText.getLocalBounds();
        m_progressText.setPosition(m_position.x + m_width - progressBounds.width - 5.0f,
            m_position.y - 25.0f);
    }

    void GrowthMeter::setStage(int stage)
    {
        m_currentStage = std::clamp(stage, 1, Constants::MAX_STAGES);  // Now supports 3 stages

        // Update max progress based on stage
        switch (m_currentStage)
        {
        case 1:
            m_maxProgress = m_stage1Progress;
            break;
        case 2:
            m_maxProgress = m_stage2Progress;
            break;
        case 3:
            m_maxProgress = m_stage3Progress;
            break;
        }

        // Update stage text
        std::ostringstream stageStream;
        stageStream << "Stage " << m_currentStage;
        m_stageText.setString(stageStream.str());

        // Update fill bar color based on stage (removed stage 4)
        sf::Color stageColors[] = {
            sf::Color(0, 255, 100),    // Green for stage 1
            sf::Color(0, 150, 255),    // Blue for stage 2
            sf::Color(255, 100, 0)     // Orange for stage 3 (final)
        };
        m_fillBar.setFillColor(stageColors[m_currentStage - 1]);

        updateVisuals();
    }

    void GrowthMeter::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_background, states);
        target.draw(m_fillBar, states);
        target.draw(m_border, states);
        target.draw(m_stageText, states);
        target.draw(m_progressText, states);
    }

    void GrowthMeter::updateVisuals()
    {
        // Update fill bar width
        float fillPercentage = m_maxProgress > 0 ? m_currentProgress / m_maxProgress : 0.0f;
        float fillWidth = (m_width - m_borderThickness * 2) * fillPercentage;
        m_fillBar.setSize(sf::Vector2f(fillWidth, m_height - m_borderThickness * 2));

        // Update progress text
        std::ostringstream progressStream;
        progressStream << std::fixed << std::setprecision(0)
            << m_currentProgress << "/" << m_maxProgress;
        m_progressText.setString(progressStream.str());

        // Reposition progress text to keep it right-aligned
        sf::FloatRect progressBounds = m_progressText.getLocalBounds();
        m_progressText.setPosition(m_position.x + m_width - progressBounds.width - 5.0f,
            m_position.y - 25.0f);
    }
}