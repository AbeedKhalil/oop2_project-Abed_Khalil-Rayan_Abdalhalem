// SmallFish.cpp
#include "SmallFish.h"
#include "GameConstants.h"

namespace FishGame
{
    using namespace Constants;

    SmallFish::SmallFish(int currentLevel)
        : Fish(FishSize::Small, m_smallFishSpeed, currentLevel)
    {
        updateVisual();
    }

    void SmallFish::updateVisual()
    {
        m_baseColor = SMALL_FISH_COLOR;
        m_outlineColor = SMALL_FISH_OUTLINE;
        m_outlineThickness = 1.0f;

        Fish::updateVisual();
    }
}