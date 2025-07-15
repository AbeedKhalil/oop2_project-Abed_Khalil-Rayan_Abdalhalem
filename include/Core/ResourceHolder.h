#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <unordered_map>
#include <cassert>
#include "GameExceptions.h"

namespace FishGame
{
    enum class Fonts
    {
        Main
    };

    // Generic resource holder template
    template<typename Resource, typename Identifier>
    class ResourceHolder
    {
    public:
        ResourceHolder() = default;
        ~ResourceHolder() = default;

        // Delete copy operations
        ResourceHolder(const ResourceHolder&) = delete;
        ResourceHolder& operator=(const ResourceHolder&) = delete;

        // Allow move operations
        ResourceHolder(ResourceHolder&&) = default;
        ResourceHolder& operator=(ResourceHolder&&) = default;

        void load(Identifier id, const std::string& filename);

        template<typename Parameter>
        void load(Identifier id, const std::string& filename, const Parameter& secondParam);

        // Insert already loaded resource (useful for async loading)
        void insert(Identifier id, std::unique_ptr<Resource> resource);

        Resource& get(Identifier id);
        const Resource& get(Identifier id) const;

        void reserve(std::size_t count);

    private:
        void insertResource(Identifier id, std::unique_ptr<Resource> resource);

        std::unordered_map<Identifier, std::unique_ptr<Resource>> m_resourceMap;
    };

    // Template implementations
template<typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::load(Identifier id, const std::string& filename)
    {
        auto resource = std::make_unique<Resource>();
        if (!resource->loadFromFile(filename))
        {
            throw ResourceLoadException("Failed to load resource: " + filename);
        }

        insertResource(id, std::move(resource));
    }

    template<typename Resource, typename Identifier>
    void ResourceHolder<Resource, Identifier>::insert(Identifier id, std::unique_ptr<Resource> resource)
    {
        insertResource(id, std::move(resource));
    }

template<typename Resource, typename Identifier>
template<typename Parameter>
void ResourceHolder<Resource, Identifier>::load(Identifier id, const std::string& filename, const Parameter& secondParam)
    {
        auto resource = std::make_unique<Resource>();
        if (!resource->loadFromFile(filename, secondParam))
        {
            throw ResourceLoadException("Failed to load resource: " + filename);
        }

        insertResource(id, std::move(resource));
    }

    template<typename Resource, typename Identifier>
    Resource& ResourceHolder<Resource, Identifier>::get(Identifier id)
    {
        auto found = m_resourceMap.find(id);
        if (found == m_resourceMap.end())
        {
            throw ResourceNotFoundException(
                "ResourceHolder::get - Resource not found");
        }

        assert(found->second != nullptr);
        return *found->second;
    }

    template<typename Resource, typename Identifier>
    const Resource& ResourceHolder<Resource, Identifier>::get(Identifier id) const
    {
        auto found = m_resourceMap.find(id);
        if (found == m_resourceMap.end())
        {
            throw ResourceNotFoundException(
                "ResourceHolder::get - Resource not found");
        }

        assert(found->second != nullptr);
        return *found->second;
    }

    template<typename Resource, typename Identifier>
    void ResourceHolder<Resource, Identifier>::insertResource(Identifier id, std::unique_ptr<Resource> resource)
    {
        auto inserted = m_resourceMap.insert(std::make_pair(id, std::move(resource)));
        if (!inserted.second)
        {
            throw ResourceInsertionException(
                "ResourceHolder::insertResource - Failed to insert resource");
        }
    }

    template<typename Resource, typename Identifier>
    void ResourceHolder<Resource, Identifier>::reserve(std::size_t count)
    {
        m_resourceMap.reserve(count);
    }

    // Type aliases for convenience
    using FontHolder = ResourceHolder<sf::Font, Fonts>;
}
