#pragma once

#include "GrowthMeter.h"
#include "FrenzySystem.h"
#include "PowerUp.h"
#include "IPowerUpManager.h"
#include "IScoreSystem.h"
#include "ScoreSystem.h"
#include <memory>
#include "BonusItemManager.h"
#include "OysterManager.h"

namespace FishGame
{
    struct GameSystems
    {
        std::unique_ptr<GrowthMeter> growthMeter;
        std::unique_ptr<FrenzySystem> frenzySystem;
        std::unique_ptr<PowerUpManager> powerUpManager;
        std::unique_ptr<ScoreSystem> scoreSystem;
        std::unique_ptr<BonusItemManager> bonusItemManager;
        std::unique_ptr<FixedOysterManager> oysterManager;

        GameSystems() = default;

        void initialize(const sf::Font& font, const sf::Vector2u& windowSize, SpriteManager& spriteManager)
        {
            growthMeter = std::make_unique<GrowthMeter>(font);
            frenzySystem = std::make_unique<FrenzySystem>(font);
            powerUpManager = std::make_unique<PowerUpManager>();
            scoreSystem = std::make_unique<ScoreSystem>(font);
            bonusItemManager = std::make_unique<BonusItemManager>(windowSize, font, spriteManager);
            oysterManager = std::make_unique<FixedOysterManager>(windowSize, spriteManager);
        }

        GrowthMeter& getGrowthMeter() { return *growthMeter; }
        FrenzySystem& getFrenzySystem() { return *frenzySystem; }
        IPowerUpManager& getPowerUpManager() { return *powerUpManager; }
        IScoreSystem& getScoreSystem() { return *scoreSystem; }
        BonusItemManager& getBonusItemManager() { return *bonusItemManager; }
        FixedOysterManager& getOysterManager() { return *oysterManager; }
    };
}
