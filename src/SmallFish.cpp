// SmallFish.cpp
#include "SmallFish.h"

namespace FishGame
{
    SmallFish::SmallFish()
        : Fish(FishSize::Small, m_smallFishSpeed, m_smallFishPoints)
    {
        updateVisual();
    }

    void SmallFish::updateVisual()
    {
        m_baseColor = sf::Color::Green;
        m_outlineColor = sf::Color(0, 100, 0); // Dark green
        m_outlineThickness = 1.0f;

        Fish::updateVisual();
    }
}