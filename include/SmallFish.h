// SmallFish.h
#pragma once

#include "Fish.h"

namespace FishGame
{
    class SmallFish : public Fish
    {
    public:
        explicit SmallFish(int currentLevel = 1);
        ~SmallFish() override = default;

        EntityType getType() const override { return EntityType::SmallFish; }

    protected:
        void updateVisual() override;

    private:
        static constexpr float m_smallFishSpeed = 150.0f;
    };
}