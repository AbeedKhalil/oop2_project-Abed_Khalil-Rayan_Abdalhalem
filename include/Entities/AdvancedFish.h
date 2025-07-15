#pragma once

#include "Fish.h"
#include "Animator.h"

namespace FishGame {

// Movement pattern strategies
enum class MovementPattern
{
    Linear,
    Sinusoidal,
    ZigZag,
    Aggressive
};

class AdvancedFish : public Fish
{
public:
    AdvancedFish(FishSize size, float speed, int currentLevel, MovementPattern pattern);
    virtual ~AdvancedFish() = default;

    void update(sf::Time deltaTime) override;
    void setMovementPattern(MovementPattern pattern) { m_movementPattern = pattern; }

protected:
    virtual void updateMovementPattern(sf::Time deltaTime);

protected:
    MovementPattern m_movementPattern;
    float m_patternTimer;
    float m_baseY;
    float m_amplitude;
    float m_frequency;
};

} // namespace FishGame
