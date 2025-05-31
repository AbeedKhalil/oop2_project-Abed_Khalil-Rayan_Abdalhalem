#include "ProgressBar.h"
#include <sstream>
#include <iomanip>

namespace FishGame
{
    using namespace Constants;

    ProgressBar::ProgressBar()
        : m_background()
        , m_fillBar()
        , m_outline()
        , m_stageText()
        , m_progressText()
        , m_currentProgress(0.0f)
        , m_maxProgress(1.0f)
        , m_currentStage(1)
        , m_position(0.0f, 0.0f)
        , m_size(PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT)
    {
        // Setup background
        m_background.setFillColor(PROGRESS_BAR_BACKGROUND);
        m_background.setSize(m_size);

        // Setup fill bar
        m_fillBar.setFillColor(PROGRESS_BAR_FILL);

        // Setup outline
        m_outline.setFillColor(sf::Color::Transparent);
        m_outline.setOutlineColor(PROGRESS_BAR_OUTLINE_COLOR);
        m_outline.setOutlineThickness(PROGRESS_BAR_OUTLINE);
        m_outline.setSize(m_size);

        // Setup texts
        m_stageText.setCharacterSize(16);
        m_stageText.setFillColor(sf::Color::White);

        m_progressText.setCharacterSize(14);
        m_progressText.setFillColor(sf::Color::White);

        updateBar();
    }

    void ProgressBar::setPosition(float x, float y)
    {
        m_position = sf::Vector2f(x, y + 10.0f);
        m_background.setPosition(m_position);
        m_fillBar.setPosition(m_position);
        m_outline.setPosition(m_position);

        // Position texts with proper spacing
        m_stageText.setPosition(x, y - 14.5f);
        m_progressText.setPosition(x + m_size.x / 2.0f, y + m_size.y + 15.0f);
    }

    void ProgressBar::setSize(float width, float height)
    {
        m_size = sf::Vector2f(width, height);
        m_background.setSize(m_size);
        m_outline.setSize(m_size);
        updateBar();
    }

    void ProgressBar::setProgress(float current, float max)
    {
        m_currentProgress = current;
        m_maxProgress = max;
        updateBar();
    }

    void ProgressBar::setStageInfo(int currentStage, int currentScore)
    {
        m_currentStage = currentStage;

        // Update stage text
        std::string stageName;
        switch (currentStage)
        {
        case 1: stageName = "Small Fish"; break;
        case 2: stageName = "Medium Fish"; break;
        case 3: stageName = "Large Fish"; break;
        }

        std::ostringstream stageStream;
        stageStream << "Stage: " << stageName;
        m_stageText.setString(stageStream.str());

        // Calculate progress within current stage
        int stageStart = (currentStage == 1) ? 0 : (currentStage == 2) ? 100 : 200;
        int stageEnd = (currentStage == 1) ? 100 : (currentStage == 2) ? 200 : 400;

        float progress = static_cast<float>(currentScore - stageStart);
        float maxProgress = static_cast<float>(stageEnd - stageStart);

        setProgress(progress, maxProgress);

        // Update progress text
        std::ostringstream progressStream;
        progressStream << std::fixed << std::setprecision(0)
            << (progress / maxProgress * 100.0f) << "%";
        m_progressText.setString(progressStream.str());

        // Center progress text
        sf::FloatRect textBounds = m_progressText.getLocalBounds();
        m_progressText.setOrigin(textBounds.width / 2.0f, 0.0f);
    }

    void ProgressBar::setFont(const sf::Font& font)
    {
        m_stageText.setFont(font);
        m_progressText.setFont(font);
    }

    void ProgressBar::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_background, states);
        target.draw(m_fillBar, states);
        target.draw(m_outline, states);
        target.draw(m_stageText, states);
        target.draw(m_progressText, states);
    }

    void ProgressBar::updateBar()
    {
        float fillWidth = (m_maxProgress > 0) ?
            (m_currentProgress / m_maxProgress) * m_size.x : 0.0f;

        fillWidth = std::max(0.0f, std::min(fillWidth, m_size.x));

        m_fillBar.setSize(sf::Vector2f(fillWidth, m_size.y));
    }
}