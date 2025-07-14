#include "SpawnController.h"
#include <algorithm>

namespace FishGame {

SpawnController::SpawnController(EnhancedFishSpawner& spawner,
                                 SpawnSystem& spawnSystem,
                                 BonusItemManager& bonusMgr,
                                 std::vector<std::unique_ptr<Entity>>& entities,
                                 std::vector<std::unique_ptr<BonusItem>>& bonus,
                                 std::vector<std::unique_ptr<Hazard>>& hazards)
    : m_spawner(spawner)
    , m_spawnSystem(spawnSystem)
    , m_bonusMgr(bonusMgr)
    , m_entities(entities)
    , m_bonusItems(bonus)
    , m_hazards(hazards) {}

void SpawnController::update(sf::Time dt, int level)
{
    m_spawner.update(dt, level);
    auto& spawnedFish = m_spawner.getSpawnedFish();
    std::move(spawnedFish.begin(), spawnedFish.end(), std::back_inserter(m_entities));
    spawnedFish.clear();

    if (m_hazardSpawnTimer.update(dt)) {
        if (auto h = m_spawnSystem.spawnRandomHazard())
            m_hazards.push_back(std::move(h));
    }

    if (m_powerUpSpawnTimer.update(dt)) {
        if (auto p = m_spawnSystem.spawnRandomPowerUp())
            m_bonusItems.push_back(std::move(p));
    }

    m_bonusMgr.update(dt);
    auto newItems = m_bonusMgr.collectSpawnedItems();
    std::move(newItems.begin(), newItems.end(), std::back_inserter(m_bonusItems));
}

} // namespace FishGame
