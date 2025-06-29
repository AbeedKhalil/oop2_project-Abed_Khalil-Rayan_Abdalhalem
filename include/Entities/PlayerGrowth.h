#pragma once

#include <SFML/System/Time.hpp>

namespace FishGame
{
    class Player;

    class PlayerGrowth
    {
    public:
        explicit PlayerGrowth(Player& player);
        void grow(int scoreValue);
        void addPoints(int points);
        void resetSize();
        void fullReset();
        void checkStageAdvancement();
        void updateStage();
    private:
        Player& m_player;
    };
}
