#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <SFML/Graphics.hpp>

namespace FishGame
{
    template<typename BaseType>
    class EntityManager
    {
    public:
        using EntityPtr = std::unique_ptr<BaseType>;
        using Container = std::vector<EntityPtr>;
        using UpdateFunc = std::function<void(BaseType&, sf::Time)>;
        using FilterFunc = std::function<bool(const BaseType&)>;

        EntityManager() = default;
        ~EntityManager() = default;

        // Delete copy operations
        EntityManager(const EntityManager&) = delete;
        EntityManager& operator=(const EntityManager&) = delete;

        // Allow move operations
        EntityManager(EntityManager&&) = default;
        EntityManager& operator=(EntityManager&&) = default;

        void add(EntityPtr entity)
        {
            if (entity)
            {
                m_entities.push_back(std::move(entity));
            }
        }

        template<typename T, typename... Args>
        T* create(Args&&... args)
        {
            auto entity = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = entity.get();
            add(std::move(entity));
            return ptr;
        }

        void update(sf::Time deltaTime, UpdateFunc updateFunc = nullptr)
        {
            // Update all entities
            std::for_each(m_entities.begin(), m_entities.end(),
                [deltaTime, &updateFunc](EntityPtr& entity) {
                    if (entity && entity->isAlive())
                    {
                        entity->update(deltaTime);
                        if (updateFunc)
                        {
                            updateFunc(*entity, deltaTime);
                        }
                    }
                });

            // Remove dead entities
            removeIf([](const BaseType& entity) { return !entity.isAlive(); });
        }

        void render(sf::RenderTarget& target) const
        {
            std::for_each(m_entities.begin(), m_entities.end(),
                [&target](const EntityPtr& entity) {
                    if (entity && entity->isAlive())
                    {
                        target.draw(*entity);
                    }
                });
        }

        void removeIf(FilterFunc predicate)
        {
            m_entities.erase(
                std::remove_if(m_entities.begin(), m_entities.end(),
                    [&predicate](const EntityPtr& entity) {
                        return !entity || predicate(*entity);
                    }),
                m_entities.end()
            );
        }

        template<typename T>
        std::vector<T*> getEntitiesOfType()
        {
            std::vector<T*> result;
            std::for_each(m_entities.begin(), m_entities.end(),
                [&result](const EntityPtr& entity) {
                    if (T* typed = dynamic_cast<T*>(entity.get()))
                    {
                        result.push_back(typed);
                    }
                });
            return result;
        }

        Container& getEntities() { return m_entities; }
        const Container& getEntities() const { return m_entities; }

        void clear() { m_entities.clear(); }
        size_t size() const { return m_entities.size(); }
        bool empty() const { return m_entities.empty(); }

    private:
        Container m_entities;
    };
}