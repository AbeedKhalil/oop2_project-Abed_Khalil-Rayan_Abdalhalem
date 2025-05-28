// LargeFish.cpp
#include "LargeFish.h"

namespace FishGame
{
    LargeFish::LargeFish()
        : Fish(FishSize::Large, m_largeFishSpeed, m_largeFishPoints)
    {
        updateVisual();
    }

    void LargeFish::updateVisual()
    {
        m_baseColor = sf::Color::Red;
        m_outlineColor = sf::Color(100, 0, 0); // Dark red
        m_outlineThickness = 2.0f;

        Fish::updateVisual();
    }
}