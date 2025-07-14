#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "SpriteManager.h"

namespace FishGame
{
    enum class ScoreEventType;

    class IScoreSystem
    {
    public:
        virtual ~IScoreSystem() = default;

        virtual void addScore(ScoreEventType type, int basePoints, sf::Vector2f position,
                              int frenzyMultiplier, float powerUpMultiplier) = 0;
        virtual void registerHit() = 0;
        virtual void registerMiss() = 0;
        virtual int getChainBonus() const = 0;
        virtual void registerTailBite(sf::Vector2f position, int frenzyMultiplier,
                                      float powerUpMultiplier) = 0;
        virtual void update(sf::Time deltaTime) = 0;
        virtual void drawFloatingScores(sf::RenderTarget& target) const = 0;
        virtual int getCurrentScore() const = 0;
        virtual void setCurrentScore(int score) = 0;
        virtual void recordFish(TextureID id) = 0;
        virtual const std::unordered_map<TextureID,int>& getFishCounts() const = 0;
        virtual void reset() = 0;
    };
}
