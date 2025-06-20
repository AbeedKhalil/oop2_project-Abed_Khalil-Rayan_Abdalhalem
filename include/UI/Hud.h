#pragma once

#include <SFML/Graphics.hpp>

namespace FishGame
{
    class Player;
    class SpriteManager;

    class Hud : public sf::Drawable
    {
    public:
        Hud(const sf::Font& font, const sf::Vector2u& windowSize, SpriteManager& spriteManager);
        ~Hud() = default;

        // Delete copy operations
        Hud(const Hud&) = delete;
        Hud& operator=(const Hud&) = delete;

        // Update HUD elements based on player state
        void update(const Player& player, int score, int lives);

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        // Initialize HUD elements
        void initializeGrowth();
        void initializeScore();
        void initializeAbility();
        void initializeLives();

        // Update individual elements
        void updateGrowth(const Player& player);
        void updateScore(int score);
        void updateAbility(const Player& player);
        void updateLives(int lives);

    private:
        const sf::Font& m_font;
        sf::Vector2u m_windowSize;
        SpriteManager* m_spriteManager;

        // Growth elements
        sf::Text m_growthLabel;
        sf::RectangleShape m_growthBarBackground;
        sf::RectangleShape m_growthBarFill;
        sf::RectangleShape m_growthBarOutline;

        // Score elements
        sf::Text m_scoreLabel;
        sf::Text m_scoreValue;

        // Ability elements
        sf::Text m_abilityLabel;
        sf::RectangleShape m_abilityBarBackground;
        sf::RectangleShape m_abilityBarFill;
        sf::RectangleShape m_abilityBarOutline;
        sf::Sprite m_abilityFishIcon;
        sf::Text m_abilityCountText;

        // Lives elements
        std::vector<sf::RectangleShape> m_lifeIcons;
        int m_currentLives;
    };
}