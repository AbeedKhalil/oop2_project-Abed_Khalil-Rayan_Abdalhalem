#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include <string>

class AnimatedSprite : public sf::Drawable
{
public:
    struct Animation
    {
        std::vector<sf::IntRect> frames;
        sf::Time frameTime{};
        bool loop{true};
    };

    explicit AnimatedSprite(const sf::Texture& texture);

    void addAnimation(const std::string& name, const Animation& anim);
    void play(const std::string& name);
    void update(sf::Time dt);

    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const { return m_sprite.getPosition(); }
    void setScale(const sf::Vector2f& scale) { m_sprite.setScale(scale); }
    void setRotation(float angle) { m_sprite.setRotation(angle); }
    float getRotation() const { return m_sprite.getRotation(); }

    bool isFinished() const { return m_finished; }

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    const sf::Texture& m_texture;
    sf::Sprite m_sprite;
    std::unordered_map<std::string, Animation> m_anims;
    const Animation* m_current{nullptr};
    std::size_t m_index{0};
    sf::Time m_elapsed{};
    bool m_finished{true};
};

