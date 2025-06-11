#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <optional>

namespace FishGame
{
    // Sprite configuration structure
    template<typename EntityType>
    struct SpriteConfig
    {
        std::string textureName;
        sf::Vector2f baseSize;
        sf::Vector2f origin;
        float scaleMultiplier = 1.0f;
        bool maintainAspectRatio = true;

        // Optional animation data for future use
        std::optional<sf::IntRect> textureRect;
        std::optional<float> rotationOffset;
    };

    // Template-based sprite component
    template<typename OwnerType>
    class SpriteComponent : public sf::Drawable
    {
    public:
        explicit SpriteComponent(OwnerType* owner);
        ~SpriteComponent() = default;

        // Initialization
        void setTexture(const sf::Texture& texture);
        void configure(const SpriteConfig<OwnerType>& config);

        // Update and positioning
        void update(sf::Time deltaTime);
        void syncWithOwner();

        // Sprite manipulation
        void setScale(float scaleX, float scaleY);
        void setScale(const sf::Vector2f& scale);
        void setColor(const sf::Color& color);
        void setRotation(float angle);

        // Getters
        const sf::Sprite& getSprite() const { return m_sprite; }
        sf::FloatRect getBounds() const { return m_sprite.getGlobalBounds(); }

        // State effects
        void applyFlashEffect(const sf::Color& color, float intensity);
        void applyPulseEffect(float scale, float speed);
        void resetEffects();

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        OwnerType* m_owner;
        sf::Sprite m_sprite;
        SpriteConfig<OwnerType> m_config;

        // Effect states
        bool m_isPulsing;
        float m_pulseScale;
        float m_pulseSpeed;
        float m_pulseTimer;

        sf::Color m_baseColor;
        sf::Color m_flashColor;
        float m_flashIntensity;
    };
}