#pragma once

#include "SpecialFish.h"
#include "GenericFish.h"
#include "CollisionDetector.h"
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <numeric>

namespace FishGame
{
    // Forward declarations
    template<typename FishType> class School;
    template<typename FishType> class SchoolMember;

    // School configuration
    struct SchoolConfig
    {
        size_t minMembers = 3;
        size_t maxMembers = 8;
        float formationRadius = 150.0f;
        float separationWeight = 1.5f;
        float alignmentWeight = 1.0f;
        float cohesionWeight = 0.8f;
        FishSize fishSize = FishSize::Small;
    };

    // Factory for creating schooling fish - must be declared before SchoolingSystem uses it
    template<typename FishType>
    class SchoolingFishFactory
    {
        static_assert(std::is_base_of_v<Fish, FishType>,
            "SchoolingFishFactory can only create Fish-derived types");

    public:
        static std::unique_ptr<SchoolMember<FishType>> create(int level = 1)
        {
            return std::make_unique<SchoolMember<FishType>>(level);
        }

        // Create from existing fish with attribute copying
        template<typename SourceFish>
        static std::unique_ptr<SchoolMember<FishType>> createFromFish(
            const SourceFish& source, int level = 1)
        {
            auto member = create(level);

            // Copy common Entity attributes
            member->setPosition(source.getPosition());
            member->setVelocity(source.getVelocity());

            // Copy Fish-specific attributes if source is a Fish
            if constexpr (std::is_base_of_v<Fish, SourceFish>)
            {
                const auto& fishSource = static_cast<const Fish&>(source);
                member->setWindowBounds(fishSource.getWindowBounds());
            }

            return member;
        }
    };

    // Template-based school management
    template<typename FishType>
    class School
    {
        static_assert(std::is_base_of_v<Fish, FishType>,
            "School can only manage Fish types");
    public:
        using MemberType = SchoolMember<FishType>;
        using MemberPtr = std::unique_ptr<MemberType>;

        explicit School(int schoolId, const SchoolConfig& config = {})
            : m_schoolId(schoolId)
            , m_config(config)
            , m_leaderIndex(0)
        {
            m_members.reserve(config.maxMembers);
        }

        // Add a new member to the school
        void addMember(MemberPtr member)
        {
            if (m_members.size() < m_config.maxMembers)
            {
                member->setSchoolId(m_schoolId);
                m_members.push_back(std::move(member));
            }
        }

        // Update school behavior
        void update(sf::Time deltaTime)
        {
            if (m_members.size() < m_config.minMembers)
                return;

            // Remove dead members
            m_members.erase(
                std::remove_if(m_members.begin(), m_members.end(),
                    [](const MemberPtr& member) { return !member || !member->isAlive(); }),
                m_members.end()
            );

            // Update leader if needed
            if (m_leaderIndex >= m_members.size())
                m_leaderIndex = 0;

            // Collect raw pointers for flocking calculations
            std::vector<MemberType*> memberPtrs;
            memberPtrs.reserve(m_members.size());

            std::transform(m_members.begin(), m_members.end(),
                std::back_inserter(memberPtrs),
                [](const MemberPtr& member) { return member.get(); });

            // Update each member's schooling behavior
            std::for_each(m_members.begin(), m_members.end(),
                [&memberPtrs, deltaTime](MemberPtr& member)
                {
                    member->updateSchooling(memberPtrs, deltaTime);
                });
        }

        // Get all members for external processing
        std::vector<std::unique_ptr<Entity>> extractMembers()
        {
            std::vector<std::unique_ptr<Entity>> entities;
            entities.reserve(m_members.size());

            for (auto& member : m_members)
            {
                entities.push_back(std::move(member));
            }

            m_members.clear();

            return entities;
        }

        size_t size() const { return m_members.size(); }
        bool isFull() const { return m_members.size() >= m_config.maxMembers; }
        bool canDisband() const { return m_members.size() < m_config.minMembers; }
        int getSchoolId() const { return m_schoolId; }

    private:
        int m_schoolId;
        SchoolConfig m_config;
        std::vector<MemberPtr> m_members;
        size_t m_leaderIndex;
    };

    // Manages multiple schools of different fish types
    class SchoolingSystem
    {
    public:
        SchoolingSystem();
        ~SchoolingSystem() = default;

        // Template method to create a new school
        template<typename FishType>
        int createSchool(const SchoolConfig& config = {})
        {
            int schoolId = m_nextSchoolId++;

            auto school = std::make_unique<School<FishType>>(schoolId, config);
            m_schools[schoolId] = std::make_unique<SchoolWrapper<FishType>>(std::move(school));

            return schoolId;
        }

        // Try to add a fish to an appropriate school
        template<typename FishType>
        bool tryAddToSchool(std::unique_ptr<FishType> fish)
        {
            // Find a school that can accept this fish type
            for (auto& [id, wrapper] : m_schools)
            {
                if (auto* typedWrapper = dynamic_cast<SchoolWrapper<FishType>*>(wrapper.get()))
                {
                    if (!typedWrapper->school->isFull())
                    {
                        // Extract current level from fish if possible
                        int level = 1; // Default level
                        if constexpr (std::is_base_of_v<Fish, FishType>)
                        {
                            level = static_cast<const Fish*>(fish.get())->getCurrentLevel();
                        }

                        // Create SchoolMember with proper constructor
                        auto member = SchoolingFishFactory<FishType>::createFromFish(*fish, level);

                        typedWrapper->school->addMember(std::move(member));
                        return true;
                    }
                }
            }

            return false;
        }

        // Overload for directly adding SchoolMember
        template<typename FishType>
        bool tryAddToSchool(std::unique_ptr<SchoolMember<FishType>> member)
        {
            // Find a school that can accept this fish type
            for (auto& [id, wrapper] : m_schools)
            {
                if (auto* typedWrapper = dynamic_cast<SchoolWrapper<FishType>*>(wrapper.get()))
                {
                    if (!typedWrapper->school->isFull())
                    {
                        typedWrapper->school->addMember(std::move(member));
                        return true;
                    }
                }
            }

            return false;
        }

        // Update all schools
        void update(sf::Time deltaTime);

        // Extract all fish from schools for rendering
        std::vector<std::unique_ptr<Entity>> extractAllFish();

        // Get statistics
        size_t getSchoolCount() const { return m_schools.size(); }
        size_t getTotalFishCount() const;

    private:
        // Base wrapper for type erasure
        struct SchoolWrapperBase
        {
            virtual ~SchoolWrapperBase() = default;
            virtual void update(sf::Time deltaTime) = 0;
            virtual std::vector<std::unique_ptr<Entity>> extractMembers() = 0;
            virtual size_t size() const = 0;
            virtual bool canDisband() const = 0;
        };

        // Template wrapper for specific fish types
        template<typename FishType>
        struct SchoolWrapper : SchoolWrapperBase
        {
            std::unique_ptr<School<FishType>> school;

            explicit SchoolWrapper(std::unique_ptr<School<FishType>> s)
                : school(std::move(s)) {}

            void update(sf::Time deltaTime) override
            {
                school->update(deltaTime);
            }

            std::vector<std::unique_ptr<Entity>> extractMembers() override
            {
                return school->extractMembers();
            }

            size_t size() const override
            {
                return school->size();
            }

            bool canDisband() const override
            {
                return school->canDisband();
            }
        };

    private:
        std::unordered_map<int, std::unique_ptr<SchoolWrapperBase>> m_schools;
        int m_nextSchoolId;

        // Template method to create default schools
        template<typename FishType>
        void createDefaultSchools(size_t count)
        {
            SchoolConfig config;

            // Set config based on fish type
            if constexpr (std::is_same_v<FishType, SmallFish>)
            {
                config.fishSize = FishSize::Small;
                config.maxMembers = 8;
            }
            else if constexpr (std::is_same_v<FishType, MediumFish>)
            {
                config.fishSize = FishSize::Medium;
                config.maxMembers = 5;
            }

            for (size_t i = 0; i < count; ++i)
            {
                createSchool<FishType>(config);
            }
        }
    };
}
