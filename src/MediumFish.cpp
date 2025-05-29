// MediumFish.cpp
#include "MediumFish.h"
#include "GameConstants.h"

namespace FishGame
{
    using namespace Constants;

    MediumFish::MediumFish(int currentLevel)
        : Fish(FishSize::Medium, m_mediumFishSpeed, currentLevel)
    {
        updateVisual();
    }

    void MediumFish::updateVisual()
    {
        m_baseColor = MEDIUM_FISH_COLOR;
        m_outlineColor = MEDIUM_FISH_OUTLINE;
        m_outlineThickness = 1.5f;

        Fish::updateVisual();
    }
}