#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include "GameConstants.h"
#include "PowerUp.h"

namespace FishGame
{
    class HUDSystem : public sf::Drawable
    {
    public:
        HUDSystem(const sf::Font& font, const sf::Vector2u& windowSize);

        void update(int score, int lives, int level, int chainBonus,
            const std::vector<PowerUpType>& activePowerUps,
            bool playerFrozen, sf::Time freezeTime,
            bool reversed, sf::Time reverseTime,
            bool stunned, sf::Time stunTime,
            float fps);

        void showMessage(const std::string& message);
        void clearMessage();

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void initText(sf::Text& text, unsigned int size, const sf::Vector2f& pos,
            const sf::Color& color = Constants::HUD_TEXT_COLOR);

        sf::Text m_scoreText;
        sf::Text m_livesText;
        sf::Text m_levelText;
        sf::Text m_chainText;
        sf::Text m_powerUpText;
        sf::Text m_effectsText;
        sf::Text m_messageText;

        const sf::Font& m_font;
        sf::Vector2u m_windowSize;
    };
}
