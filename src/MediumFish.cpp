// MediumFish.cpp
#include "MediumFish.h"

namespace FishGame
{
    MediumFish::MediumFish()
        : Fish(FishSize::Medium, m_mediumFishSpeed, m_mediumFishPoints)
    {
        updateVisual();
    }

    void MediumFish::updateVisual()
    {
        m_baseColor = sf::Color::Blue;
        m_outlineColor = sf::Color(0, 0, 100); // Dark blue
        m_outlineThickness = 1.5f;

        Fish::updateVisual();
    }
}