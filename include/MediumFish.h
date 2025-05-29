// MediumFish.h
#pragma once

#include "Fish.h"

namespace FishGame
{
    class MediumFish : public Fish
    {
    public:
        explicit MediumFish(int currentLevel = 1);
        ~MediumFish() override = default;

        EntityType getType() const override { return EntityType::MediumFish; }

    protected:
        void updateVisual() override;

    private:
        static constexpr float m_mediumFishSpeed = 120.0f;
    };
}