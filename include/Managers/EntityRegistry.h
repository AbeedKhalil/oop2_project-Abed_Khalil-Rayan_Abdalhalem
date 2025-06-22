#pragma once

#include "Entity.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace FishGame {
    // Simple runtime entity factory registry
    class EntityRegistry {
    public:
        template<typename T>
        void registerType(const std::string& name) {
            static_assert(std::is_base_of_v<Entity, T>, "T must derive from Entity");
            m_factories[name] = [] { return std::make_unique<T>(); };
        }

        std::unique_ptr<Entity> create(const std::string& name) const {
            auto it = m_factories.find(name);
            if (it != m_factories.end())
                return it->second();
            return nullptr;
        }

    private:
        std::unordered_map<std::string, std::function<std::unique_ptr<Entity>()>> m_factories;
    };
}
