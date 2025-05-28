// MediumFish.h
#pragma once

#include "Fish.h"

namespace FishGame
{
    class MediumFish : public Fish
    {
    public:
        MediumFish();
        ~MediumFish() override = default;

        EntityType getType() const override { return EntityType::MediumFish; }

    protected:
        void updateVisual() override;

    private:
        static constexpr float m_mediumFishSpeed = 120.0f;
        static constexpr int m_mediumFishPoints = 6;
    };
}