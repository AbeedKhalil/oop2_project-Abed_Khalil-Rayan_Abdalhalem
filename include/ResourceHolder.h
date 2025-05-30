#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <map>
#include <string>
#include <stdexcept>
#include <cassert>

namespace FishGame
{
    // Resource identifiers
    enum class Textures
    {
        Player,
        SmallFish,
        MediumFish,
        LargeFish,
        Background
    };

    enum class Fonts
    {
        Main,
        Score
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

        bool load(Identifier id, const std::string& filename);

        template<typename Parameter>
        bool load(Identifier id, const std::string& filename, const Parameter& secondParam);

        Resource& get(Identifier id);
        const Resource& get(Identifier id) const;

    private:
        void insertResource(Identifier id, std::unique_ptr<Resource> resource);

    private:
        std::map<Identifier, std::unique_ptr<Resource>> m_resourceMap;
    };

    // Template implementations
    template<typename Resource, typename Identifier>
    bool ResourceHolder<Resource, Identifier>::load(Identifier id, const std::string& filename)
    {
        auto resource = std::make_unique<Resource>();
        if (!resource->loadFromFile(filename))
        {
            return false;
        }

        insertResource(id, std::move(resource));
        return true;
    }

    template<typename Resource, typename Identifier>
    template<typename Parameter>
    bool ResourceHolder<Resource, Identifier>::load(Identifier id, const std::string& filename, const Parameter& secondParam)
    {
        auto resource = std::make_unique<Resource>();
        if (!resource->loadFromFile(filename, secondParam))
        {
            return false;
        }

        insertResource(id, std::move(resource));
        return true;
    }

    template<typename Resource, typename Identifier>
    Resource& ResourceHolder<Resource, Identifier>::get(Identifier id)
    {
        auto found = m_resourceMap.find(id);
        if (found == m_resourceMap.end())
        {
            throw std::runtime_error("ResourceHolder::get - Resource not found");
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
            throw std::runtime_error("ResourceHolder::get - Resource not found");
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
            throw std::runtime_error("ResourceHolder::insertResource - Failed to insert resource");
        }
    }

    // Type aliases for convenience
    using TextureHolder = ResourceHolder<sf::Texture, Textures>;
    using FontHolder = ResourceHolder<sf::Font, Fonts>;
}