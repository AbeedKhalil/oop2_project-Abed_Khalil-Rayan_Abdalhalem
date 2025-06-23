#include "SchoolingSystem.h"
#include <algorithm>
#include <numeric>

namespace FishGame
{
    SchoolingSystem::SchoolingSystem()
        : m_schools()
        , m_nextSchoolId(1)
    {
        // Create initial schools for different fish types
        createDefaultSchools<SmallFish>(1);    // Reduced from 2 to 1
        createDefaultSchools<MediumFish>(1);   // Reduced from 2 to 1
    }

    void SchoolingSystem::update(sf::Time deltaTime)
    {
        // Update all schools
        std::for_each(m_schools.begin(), m_schools.end(),
            [deltaTime](auto& pair)
            {
                pair.second->update(deltaTime);
            });

        // Remove disbanded schools
        std::erase_if(m_schools,
            [](const auto& pair)
            {
                return pair.second->canDisband();
            });
    }

    std::vector<std::unique_ptr<Entity>> SchoolingSystem::extractAllFish()
    {
        std::vector<std::unique_ptr<Entity>> allFish;

        // Calculate total size needed
        size_t totalSize = std::accumulate(m_schools.begin(), m_schools.end(), 0,
            [](size_t sum, const auto& pair)
            {
                return sum + pair.second->size();
            });

        allFish.reserve(totalSize);

        // Extract from each school
        std::for_each(m_schools.begin(), m_schools.end(),
            [&allFish](auto& pair)
            {
                auto members = pair.second->extractMembers();
                std::move(members.begin(), members.end(), std::back_inserter(allFish));
            });

        // Clear empty schools
        m_schools.clear();

        return allFish;
    }

    size_t SchoolingSystem::getTotalFishCount() const
    {
        return std::accumulate(m_schools.begin(), m_schools.end(), size_t(0),
            [](size_t sum, const auto& pair)
            {
                return sum + pair.second->size();
            });
    }
}
