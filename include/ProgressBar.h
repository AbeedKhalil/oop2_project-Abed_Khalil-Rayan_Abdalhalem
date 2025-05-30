#pragma once

#include <SFML/Graphics.hpp>
#include "GameConstants.h"

namespace FishGame
{
    class ProgressBar : public sf::Drawable
    {
    public:
        ProgressBar();
        ~ProgressBar() = default;

        void setPosition(float x, float y);
        void setSize(float width, float height);
        void setProgress(float current, float max);
        void setStageInfo(int currentStage, int currentScore);
        void setFont(const sf::Font& font);

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updateBar();

    private:
        sf::RectangleShape m_background;
        sf::RectangleShape m_fillBar;
        sf::RectangleShape m_outline;
        sf::Text m_stageText;
        sf::Text m_progressText;

        float m_currentProgress;
        float m_maxProgress;
        int m_currentStage;
        sf::Vector2f m_position;
        sf::Vector2f m_size;
    };
}