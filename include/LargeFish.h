// LargeFish.h
#pragma once

#include "Fish.h"

namespace FishGame
{
    class LargeFish : public Fish
    {
    public:
        LargeFish();
        ~LargeFish() override = default;

        EntityType getType() const override { return EntityType::LargeFish; }

    protected:
        void updateVisual() override;

    private:
        static constexpr float m_largeFishSpeed = 90.0f;
        static constexpr int m_largeFishPoints = 9;
    };
}