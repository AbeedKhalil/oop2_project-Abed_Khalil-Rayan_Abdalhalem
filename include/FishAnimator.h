#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <vector>

class FishAnimator : public sf::Drawable
{
public:
    explicit FishAnimator(const sf::Texture& texture);

    void play(const std::string& name);
    void update(sf::Time dt);

    void setPosition(sf::Vector2f pos) { m_sprite.setPosition(pos); }
    sf::Vector2f getPosition() const { return m_sprite.getPosition(); }
    void setScale(const sf::Vector2f& scale);
    sf::Vector2f getScale() const { return m_scale; }
    void setColor(const sf::Color& color) { m_sprite.setColor(color); }

private:
    struct Clip
    {
        std::vector<sf::IntRect> frames;
        sf::Time frameTime{};
        bool loop{ true };
        bool flipped{ false };
    };

    void buildAnimations();
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    const sf::Texture& m_texture;
    sf::Sprite m_sprite;
    sf::Vector2f m_scale{ 1.f, 1.f };
    std::unordered_map<std::string, Clip> m_clips;
    const Clip* m_current{ nullptr };
    std::string m_currentName;
    std::size_t m_index{ 0 };
    sf::Time m_elapsed{};

    void applyScale();
};