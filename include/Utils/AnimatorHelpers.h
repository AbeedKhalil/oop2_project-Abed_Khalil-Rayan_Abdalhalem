#pragma once

#include "Animator.h"
#include <SFML/System.hpp>
#include <string>
#include <cmath>

namespace FishGame::AnimatorHelpers {
    inline void updateTurn(
        Animator& animator,
        sf::Time dt,
        const sf::Vector2f& velocity,
        const sf::Vector2f& position,
        bool& facingRight,
        bool& turning,
        sf::Time& turnTimer,
        sf::Time turnDuration,
        std::string& currentAnimation)
    {
        bool newFacingRight = velocity.x > 0.f;
        if (std::abs(velocity.x) > 1.f && newFacingRight != facingRight)
        {
            facingRight = newFacingRight;
            std::string turn = facingRight ? "turnLeftToRight" : "turnRightToLeft";
            animator.play(turn);
            currentAnimation = turn;
            turning = true;
            turnTimer = sf::Time::Zero;
        }

        animator.update(dt);

        if (turning)
        {
            turnTimer += dt;
            if (turnTimer.asSeconds() >= turnDuration.asSeconds())
            {
                std::string swim = facingRight ? "swimRight" : "swimLeft";
                animator.play(swim);
                currentAnimation = swim;
                turning = false;
            }
        }

        animator.setPosition(position);
    }
}
