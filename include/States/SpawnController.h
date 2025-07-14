#pragma once

#include "EnhancedFishSpawner.h"
#include "SpawnSystem.h"
#include "BonusItemManager.h"
#include "Entity.h"
#include "Hazard.h"
#include <memory>
#include <vector>
#include <random>
#include "Utils/SpawnTimer.h"

namespace FishGame {

class SpawnController {
public:
    SpawnController(EnhancedFishSpawner& spawner,
                    SpawnSystem& spawnSystem,
                    BonusItemManager& bonusMgr,
                    std::vector<std::unique_ptr<Entity>>& entities,
                    std::vector<std::unique_ptr<BonusItem>>& bonus,
                    std::vector<std::unique_ptr<Hazard>>& hazards);

    void update(sf::Time dt, int level);

private:
    EnhancedFishSpawner& m_spawner;
    SpawnSystem& m_spawnSystem;
    BonusItemManager& m_bonusMgr;
    std::vector<std::unique_ptr<Entity>>& m_entities;
    std::vector<std::unique_ptr<BonusItem>>& m_bonusItems;
    std::vector<std::unique_ptr<Hazard>>& m_hazards;

    SpawnTimer<sf::Time> m_hazardSpawnTimer{sf::seconds(8.f)};
    SpawnTimer<sf::Time> m_powerUpSpawnTimer{sf::seconds(15.f)};
};

} // namespace FishGame
