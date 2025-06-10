#include "SpriteComponent.h"
#include "Entity.h"
#include "Fish.h"
#include "Player.h"
#include <cmath>
#include <algorithm>

namespace FishGame
{
    template<typename OwnerType>
    SpriteComponent<OwnerType>::SpriteComponent(OwnerType* owner)
        : m_owner(owner)
        , m_sprite()
        , m_config()
        , m_isPulsing(false)
        , m_pulseScale(1.0f)
        , m_pulseSpeed(1.0f)
        , m_pulseTimer(0.0f)
        , m_baseColor(sf::Color::White)
        , m_flashColor(sf::Color::White)
        , m_flashIntensity(0.0f)
    {
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::setTexture(const sf::Texture& texture)
    {
        m_sprite.setTexture(texture);

        // Apply texture rect if specified
        if (m_config.textureRect.has_value())
        {
            m_sprite.setTextureRect(m_config.textureRect.value());
        }

        // Set origin to center by default
        sf::FloatRect bounds = m_sprite.getLocalBounds();
        m_sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);

        // Apply custom origin if specified
        if (m_config.origin.x != 0 || m_config.origin.y != 0)
        {
            m_sprite.setOrigin(m_config.origin);
        }
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::configure(const SpriteConfig<OwnerType>& config)
    {
        m_config = config;

        // Apply base scale
        if (m_config.maintainAspectRatio)
        {
            float scale = m_config.scaleMultiplier;
            m_sprite.setScale(scale, scale);
        }
        else
        {
            m_sprite.setScale(m_config.baseSize.x / m_sprite.getLocalBounds().width,
                m_config.baseSize.y / m_sprite.getLocalBounds().height);
        }

        // Apply rotation offset if specified
        if (m_config.rotationOffset.has_value())
        {
            m_sprite.setRotation(m_config.rotationOffset.value());
        }
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::update(sf::Time deltaTime)
    {
        // Update position from owner
        syncWithOwner();

        // Update pulse effect
        if (m_isPulsing)
        {
            m_pulseTimer += deltaTime.asSeconds() * m_pulseSpeed;
            float scale = 1.0f + m_pulseScale * std::sin(m_pulseTimer);
            m_sprite.setScale(scale * m_config.scaleMultiplier,
                scale * m_config.scaleMultiplier);
        }

        // Update flash effect
        if (m_flashIntensity > 0.0f)
        {
            m_flashIntensity = std::max(0.0f, m_flashIntensity - deltaTime.asSeconds() * 2.0f);

            sf::Color currentColor;
            currentColor.r = static_cast<sf::Uint8>(
                m_baseColor.r + (m_flashColor.r - m_baseColor.r) * m_flashIntensity);
            currentColor.g = static_cast<sf::Uint8>(
                m_baseColor.g + (m_flashColor.g - m_baseColor.g) * m_flashIntensity);
            currentColor.b = static_cast<sf::Uint8>(
                m_baseColor.b + (m_flashColor.b - m_baseColor.b) * m_flashIntensity);
            currentColor.a = m_baseColor.a;

            m_sprite.setColor(currentColor);
        }
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::syncWithOwner()
    {
        if (m_owner)
        {
            // Sync position with owner entity
            if constexpr (std::is_base_of_v<Entity, OwnerType>)
            {
                m_sprite.setPosition(m_owner->getPosition());
            }
        }
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::setScale(float scaleX, float scaleY)
    {
        m_sprite.setScale(scaleX, scaleY);
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::setScale(const sf::Vector2f& scale)
    {
        m_sprite.setScale(scale);
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::setColor(const sf::Color& color)
    {
        m_baseColor = color;
        m_sprite.setColor(color);
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::setRotation(float angle)
    {
        m_sprite.setRotation(angle);
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::applyFlashEffect(const sf::Color& color, float intensity)
    {
        m_flashColor = color;
        m_flashIntensity = std::clamp(intensity, 0.0f, 1.0f);
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::applyPulseEffect(float scale, float speed)
    {
        m_isPulsing = true;
        m_pulseScale = scale;
        m_pulseSpeed = speed;
        m_pulseTimer = 0.0f;
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::resetEffects()
    {
        m_isPulsing = false;
        m_flashIntensity = 0.0f;
        m_sprite.setColor(m_baseColor);
        m_sprite.setScale(m_config.scaleMultiplier, m_config.scaleMultiplier);
    }

    template<typename OwnerType>
    void SpriteComponent<OwnerType>::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_sprite, states);
    }

    // Explicit template instantiations
    template class SpriteComponent<Entity>;
    template class SpriteComponent<Fish>;
    template class SpriteComponent<Player>;
}