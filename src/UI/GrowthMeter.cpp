#include "GrowthMeter.h"
#include "GameConstants.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <array>

namespace FishGame
{
    GrowthMeter::GrowthMeter(const sf::Font& font)
        : m_background(sf::Vector2f(m_width, m_height))
        , m_fillBar(sf::Vector2f(0.0f, m_height - m_borderThickness * 2))
        , m_border(sf::Vector2f(m_width, m_height))
        , m_stageText()
        , m_progressText()
        , m_currentProgress(0.0f)
        , m_maxProgress(static_cast<float>(Constants::POINTS_FOR_STAGE_2))
        , m_currentStage(1)
        , m_position(0.0f, 0.0f)
        , m_points(0)
        , m_targetProgress(0.0f)
        , m_glowIntensity(0.0f)
        , m_onStageComplete(nullptr)
    {
        // Setup background
        sf::Color bgColor = Constants::PROGRESS_BAR_BACKGROUND;
        bgColor.a = 200;
        m_background.setFillColor(bgColor);
        m_background.setOutlineThickness(0);

        // Setup fill bar
        m_fillBar.setFillColor(Constants::PROGRESS_BAR_FILL);
        m_fillBar.setPosition(m_borderThickness, m_borderThickness);

        // Setup border
        m_border.setFillColor(sf::Color::Transparent);
        m_border.setOutlineColor(Constants::PROGRESS_BAR_OUTLINE_COLOR);
        m_border.setOutlineThickness(m_borderThickness);

        // Setup stage text
        m_stageText.setFont(font);
        m_stageText.setCharacterSize(20);
        m_stageText.setFillColor(Constants::HUD_TEXT_COLOR);
        m_stageText.setString("Stage 1");

        // Setup progress text
        m_progressText.setFont(font);
        m_progressText.setCharacterSize(16);
        m_progressText.setFillColor(Constants::HUD_TEXT_COLOR);

        updateVisuals();
    }

    void GrowthMeter::setPoints(int points)
    {
        m_points = points;

        if (m_currentStage == 1)
        {
            m_currentProgress = static_cast<float>(points);
            m_targetProgress = m_currentProgress;
            m_maxProgress = static_cast<float>(Constants::POINTS_FOR_STAGE_2);
        }
        else if (m_currentStage == 2)
        {
            m_currentProgress = static_cast<float>(points - Constants::POINTS_FOR_STAGE_2);
            m_targetProgress = m_currentProgress;
            m_maxProgress = static_cast<float>(Constants::POINTS_FOR_STAGE_3 - Constants::POINTS_FOR_STAGE_2);
        }
        else if (m_currentStage == 3)
        {
            m_currentProgress = static_cast<float>(points - Constants::POINTS_FOR_STAGE_3);
            m_targetProgress = m_currentProgress;
            m_maxProgress = static_cast<float>(Constants::POINTS_TO_WIN - Constants::POINTS_FOR_STAGE_3);
        }

        updateVisuals();
    }

    void GrowthMeter::update(sf::Time deltaTime)
    {
        if (m_currentProgress < m_targetProgress)
        {
            float increment = m_fillSpeed * deltaTime.asSeconds();
            m_currentProgress = std::min(m_currentProgress + increment, m_targetProgress);
            updateVisuals();
        }

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
        m_points = 0;
        updateVisuals();
    }

    void GrowthMeter::setPosition(float x, float y)
    {
        m_position = sf::Vector2f(x, y);
        m_background.setPosition(m_position);
        m_fillBar.setPosition(m_position.x + m_borderThickness, m_position.y + m_borderThickness);
        m_border.setPosition(m_position);

        m_stageText.setPosition(m_position.x + 5.0f, m_position.y - 25.0f);

        sf::FloatRect progressBounds = m_progressText.getLocalBounds();
        m_progressText.setPosition(m_position.x + m_width - progressBounds.width - 5.0f,
            m_position.y - 25.0f);
    }

    void GrowthMeter::setStage(int stage)
    {
        m_currentStage = std::clamp(stage, 1, Constants::MAX_STAGES);

        std::ostringstream stageStream;
        stageStream << "Stage " << m_currentStage;
        m_stageText.setString(stageStream.str());

        const std::array<sf::Color, 3> stageColors{
            sf::Color(0, 255, 100),
            sf::Color(0, 150, 255),
            sf::Color(255, 100, 0)
        };
        m_fillBar.setFillColor(stageColors[m_currentStage - 1]);

        setPoints(m_points);
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
        float fillPercentage = m_maxProgress > 0 ? m_currentProgress / m_maxProgress : 0.0f;
        float fillWidth = (m_width - m_borderThickness * 2) * fillPercentage;
        m_fillBar.setSize(sf::Vector2f(fillWidth, m_height - m_borderThickness * 2));

        std::ostringstream progressStream;

        int targetPoints = 0;
        if (m_currentStage == 1)
            targetPoints = Constants::POINTS_FOR_STAGE_2;
        else if (m_currentStage == 2)
            targetPoints = Constants::POINTS_FOR_STAGE_3;
        else if (m_currentStage == 3)
            targetPoints = Constants::POINTS_TO_WIN;

        progressStream << "Points: " << m_points << "/" << targetPoints;
        m_progressText.setString(progressStream.str());

        sf::FloatRect progressBounds = m_progressText.getLocalBounds();
        m_progressText.setPosition(m_position.x + m_width - progressBounds.width - 5.0f,
            m_position.y - 25.0f);
    }
}