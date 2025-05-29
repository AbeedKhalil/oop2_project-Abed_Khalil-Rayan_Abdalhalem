// LargeFish.cpp
#include "LargeFish.h"
#include "GameConstants.h"

namespace FishGame
{
    using namespace Constants;

    LargeFish::LargeFish(int currentLevel)
        : Fish(FishSize::Large, m_largeFishSpeed, currentLevel)
    {
        updateVisual();
    }

    void LargeFish::updateVisual()
    {
        m_baseColor = LARGE_FISH_COLOR;
        m_outlineColor = LARGE_FISH_OUTLINE;
        m_outlineThickness = 2.0f;

        Fish::updateVisual();
    }
}