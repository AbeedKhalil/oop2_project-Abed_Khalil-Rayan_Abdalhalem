#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <vector>

class Animator : public sf::Drawable
{
public:
    Animator(const sf::Texture& texture, int frameWidth, int frameHeight, int startX = 1);

    void addClip(const std::string& name, const std::vector<sf::IntRect>& frames,
        sf::Time frameTime, bool loop = true, bool flipped = false);
    void addClipRow(const std::string& name, int rowY, int startFrame, int count,
        sf::Time frameTime, bool loop = true, bool reverse = false);
    void copyFlip(const std::string& left, const std::string& right);

    void play(const std::string& name);
    void update(sf::Time dt);

    void setPosition(sf::Vector2f pos) { m_sprite.setPosition(pos); }
    sf::Vector2f getPosition() const { return m_sprite.getPosition(); }
    void setScale(const sf::Vector2f& scale);
    sf::Vector2f getScale() const { return m_scale; }
    void setColor(const sf::Color& color) { m_sprite.setColor(color); }
    sf::FloatRect getBounds() const { return m_sprite.getGlobalBounds(); }

private:
    struct Clip
    {
        std::vector<sf::IntRect> frames;
        sf::Time frameTime{};
        bool loop{ true };
        bool flipped{ false };
    };

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void applyScale();

    const sf::Texture& m_texture;
    sf::Sprite m_sprite;
    int m_startX;
    int m_frameW;
    int m_frameH;
    sf::Vector2f m_scale{ 1.f, 1.f };
    std::unordered_map<std::string, Clip> m_clips;
    const Clip* m_current{ nullptr };
    std::string m_currentName;
    std::size_t m_index{ 0 };
    sf::Time m_elapsed{};
};

// Helper factory functions
Animator createFishAnimator(const sf::Texture& texture);
Animator createBarracudaAnimator(const sf::Texture& texture);
Animator createSimpleFishAnimator(const sf::Texture& texture);
Animator createMediumFishAnimator(const sf::Texture& texture);
Animator createPufferfishAnimator(const sf::Texture& texture);
