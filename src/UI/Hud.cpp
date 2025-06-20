#include "Hud.h"
#include "Player.h"
#include "SpriteManager.h"
#include "GameConstants.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace FishGame
{
    // HUD layout constants
    namespace HudConstants
    {
        constexpr float HUD_Y_POSITION = 8.0f;
        constexpr float LABEL_SIZE = 20.0f;
        constexpr float VALUE_SIZE = 24.0f;

        // Growth section
        constexpr float GROWTH_X = 20.0f;
        constexpr float GROWTH_BAR_WIDTH = 300.0f;
        constexpr float GROWTH_BAR_HEIGHT = 12.0f;
        constexpr float GROWTH_BAR_OFFSET = 10.0f;

        // Score section
        constexpr float SCORE_RIGHT_MARGIN = 260.0f;
        constexpr float SCORE_VALUE_Y_OFFSET = 25.0f;

        // Ability section
        constexpr float ABILITY_Y_OFFSET = 40.0f;
        constexpr float ABILITY_BAR_WIDTH = 220.0f;
        constexpr float ABILITY_BAR_HEIGHT = 10.0f;
        constexpr float ABILITY_FISH_WIDTH = 24.0f;
        constexpr float ABILITY_FISH_HEIGHT = 12.0f;
        constexpr float ABILITY_FISH_OFFSET = 5.0f;

        // Lives section
        constexpr float LIFE_ICON_SIZE = 32.0f;
        constexpr float LIFE_ICON_SPACING = 5.0f;
        constexpr float LIFE_RIGHT_MARGIN = 20.0f;
        constexpr float LIFE_CORNER_RADIUS = 6.0f;

        // Colors
        const sf::Color LABEL_COLOR(255, 255, 150); // Light yellow
        const sf::Color BAR_BACKGROUND_COLOR(30, 30, 30);
        const sf::Color BAR_OUTLINE_COLOR(200, 200, 200);
        const sf::Color GROWTH_FILL_COLOR(100, 255, 100);
        const sf::Color ABILITY_FILL_COLOR(0, 255, 0);
        const sf::Color LIFE_ICON_COLOR(255, 100, 100);
    }

    using namespace HudConstants;

    Hud::Hud(const sf::Font& font, const sf::Vector2u& windowSize, SpriteManager& spriteManager)
        : m_font(font)
        , m_windowSize(windowSize)
        , m_spriteManager(&spriteManager)
        , m_currentLives(0)
    {
        initializeGrowth();
        initializeScore();
        initializeAbility();
        initializeLives();
    }

    void Hud::initializeGrowth()
    {
        // Growth label
        m_growthLabel.setFont(m_font);
        m_growthLabel.setString("GROWTH");
        m_growthLabel.setCharacterSize(static_cast<unsigned int>(LABEL_SIZE));
        m_growthLabel.setFillColor(LABEL_COLOR);
        m_growthLabel.setStyle(sf::Text::Italic);
        m_growthLabel.setPosition(GROWTH_X, HUD_Y_POSITION);

        // Calculate bar position
        float barX = GROWTH_X + m_growthLabel.getGlobalBounds().width + GROWTH_BAR_OFFSET;

        // Growth bar background
        m_growthBarBackground.setSize(sf::Vector2f(GROWTH_BAR_WIDTH, GROWTH_BAR_HEIGHT));
        m_growthBarBackground.setPosition(barX, HUD_Y_POSITION + 4.0f);
        m_growthBarBackground.setFillColor(BAR_BACKGROUND_COLOR);

        // Growth bar fill
        m_growthBarFill.setSize(sf::Vector2f(0.0f, GROWTH_BAR_HEIGHT));
        m_growthBarFill.setPosition(barX, HUD_Y_POSITION + 4.0f);
        m_growthBarFill.setFillColor(GROWTH_FILL_COLOR);

        // Growth bar outline
        m_growthBarOutline.setSize(sf::Vector2f(GROWTH_BAR_WIDTH, GROWTH_BAR_HEIGHT));
        m_growthBarOutline.setPosition(barX, HUD_Y_POSITION + 4.0f);
        m_growthBarOutline.setFillColor(sf::Color::Transparent);
        m_growthBarOutline.setOutlineColor(BAR_OUTLINE_COLOR);
        m_growthBarOutline.setOutlineThickness(1.0f);
    }

    void Hud::initializeScore()
    {
        // Score label position (flush-right minus margin)
        float scoreX = static_cast<float>(m_windowSize.x) - SCORE_RIGHT_MARGIN;

        // Score label
        m_scoreLabel.setFont(m_font);
        m_scoreLabel.setString("SCORE");
        m_scoreLabel.setCharacterSize(static_cast<unsigned int>(LABEL_SIZE));
        m_scoreLabel.setFillColor(LABEL_COLOR);
        m_scoreLabel.setStyle(sf::Text::Italic);

        // Right-align the label
        sf::FloatRect labelBounds = m_scoreLabel.getLocalBounds();
        m_scoreLabel.setOrigin(labelBounds.width, 0.0f);
        m_scoreLabel.setPosition(scoreX, HUD_Y_POSITION);

        // Score value (beneath label)
        m_scoreValue.setFont(m_font);
        m_scoreValue.setString("0");
        m_scoreValue.setCharacterSize(static_cast<unsigned int>(VALUE_SIZE));
        m_scoreValue.setFillColor(sf::Color::White);
        m_scoreValue.setPosition(scoreX, HUD_Y_POSITION + SCORE_VALUE_Y_OFFSET);
    }

    void Hud::initializeAbility()
    {
        // Ability section position (below score)
        float abilityX = static_cast<float>(m_windowSize.x) - SCORE_RIGHT_MARGIN;
        float abilityY = HUD_Y_POSITION + ABILITY_Y_OFFSET;

        // Ability label
        m_abilityLabel.setFont(m_font);
        m_abilityLabel.setString("ABILITY");
        m_abilityLabel.setCharacterSize(static_cast<unsigned int>(LABEL_SIZE));
        m_abilityLabel.setFillColor(LABEL_COLOR);
        m_abilityLabel.setStyle(sf::Text::Italic);

        sf::FloatRect labelBounds = m_abilityLabel.getLocalBounds();
        m_abilityLabel.setOrigin(labelBounds.width, 0.0f);
        m_abilityLabel.setPosition(abilityX, abilityY);

        // Calculate bar position (left-aligned under label)
        float barX = abilityX - ABILITY_BAR_WIDTH;
        float barY = abilityY + 25.0f;

        // Ability bar background
        m_abilityBarBackground.setSize(sf::Vector2f(ABILITY_BAR_WIDTH, ABILITY_BAR_HEIGHT));
        m_abilityBarBackground.setPosition(barX, barY);
        m_abilityBarBackground.setFillColor(sf::Color::Black);

        // Ability bar fill
        m_abilityBarFill.setSize(sf::Vector2f(0.0f, ABILITY_BAR_HEIGHT));
        m_abilityBarFill.setPosition(barX, barY);
        m_abilityBarFill.setFillColor(ABILITY_FILL_COLOR);

        // Ability bar outline
        m_abilityBarOutline.setSize(sf::Vector2f(ABILITY_BAR_WIDTH, ABILITY_BAR_HEIGHT));
        m_abilityBarOutline.setPosition(barX, barY);
        m_abilityBarOutline.setFillColor(sf::Color::Transparent);
        m_abilityBarOutline.setOutlineColor(BAR_OUTLINE_COLOR);
        m_abilityBarOutline.setOutlineThickness(1.0f);

        // Fish icon sprite (placeholder - will be set when texture is available)
        m_abilityFishIcon.setPosition(abilityX + ABILITY_FISH_OFFSET, barY - 1.0f);

        // Try to set fish texture
        try {
            m_abilityFishIcon.setTexture(m_spriteManager->getTexture(TextureID::SmallFish));
            m_abilityFishIcon.setScale(
                ABILITY_FISH_WIDTH / m_abilityFishIcon.getLocalBounds().width,
                ABILITY_FISH_HEIGHT / m_abilityFishIcon.getLocalBounds().height
            );
        }
        catch (...) {
            // Texture not loaded yet
        }

        // Ability count text
        m_abilityCountText.setFont(m_font);
        m_abilityCountText.setString("x 0");
        m_abilityCountText.setCharacterSize(16);
        m_abilityCountText.setFillColor(sf::Color::White);
        m_abilityCountText.setPosition(
            abilityX + ABILITY_FISH_OFFSET + ABILITY_FISH_WIDTH + 5.0f,
            barY - 3.0f
        );
    }

    void Hud::initializeLives()
    {
        // Reserve space for maximum lives
        m_lifeIcons.reserve(5);

        // Create rounded rectangle for life icons
        for (int i = 0; i < 5; ++i)
        {
            sf::RectangleShape lifeIcon(sf::Vector2f(LIFE_ICON_SIZE, LIFE_ICON_SIZE));
            lifeIcon.setFillColor(LIFE_ICON_COLOR);
            lifeIcon.setOutlineColor(sf::Color::White);
            lifeIcon.setOutlineThickness(2.0f);

            // Position from right to left
            float x = static_cast<float>(m_windowSize.x) - LIFE_RIGHT_MARGIN -
                (LIFE_ICON_SIZE + LIFE_ICON_SPACING) * (i + 1);
            lifeIcon.setPosition(x, HUD_Y_POSITION);

            m_lifeIcons.push_back(lifeIcon);
        }
    }

    void Hud::update(const Player& player, int score, int lives)
    {
        updateGrowth(player);
        updateScore(score);
        updateAbility(player);
        updateLives(lives);
    }

    void Hud::updateGrowth(const Player& player)
    {
        // Calculate growth ratio based on points progress
        float growthRatio = 0.0f;
        int points = player.getPoints();
        int stage = player.getCurrentStage();

        if (stage == 1) {
            growthRatio = static_cast<float>(points) / Constants::POINTS_FOR_STAGE_2;
        }
        else if (stage == 2) {
            int stagePoints = points - Constants::POINTS_FOR_STAGE_2;
            int stageTotal = Constants::POINTS_FOR_STAGE_3 - Constants::POINTS_FOR_STAGE_2;
            growthRatio = static_cast<float>(stagePoints) / stageTotal;
        }
        else if (stage == 3) {
            int stagePoints = points - Constants::POINTS_FOR_STAGE_3;
            int stageTotal = Constants::POINTS_TO_WIN - Constants::POINTS_FOR_STAGE_3;
            growthRatio = static_cast<float>(stagePoints) / stageTotal;
        }

        growthRatio = std::clamp(growthRatio, 0.0f, 1.0f);

        // Update fill bar width
        float fillWidth = GROWTH_BAR_WIDTH * growthRatio;
        m_growthBarFill.setSize(sf::Vector2f(fillWidth, GROWTH_BAR_HEIGHT));
    }

    void Hud::updateScore(int score)
    {
        // Update score text
        std::ostringstream scoreStream;
        scoreStream << std::setw(7) << std::setfill('0') << score;
        m_scoreValue.setString(scoreStream.str());

        // Right-align the value
        sf::FloatRect valueBounds = m_scoreValue.getLocalBounds();
        m_scoreValue.setOrigin(valueBounds.width, 0.0f);
    }

    void Hud::updateAbility(const Player& player)
    {
        // Placeholder implementation - ability system not yet implemented
        // For now, show empty bar with 0 uses
        float chargeRatio = 0.0f;
        int remainingUses = 0;

        // Update fill bar width
        float fillWidth = ABILITY_BAR_WIDTH * chargeRatio;
        m_abilityBarFill.setSize(sf::Vector2f(fillWidth, ABILITY_BAR_HEIGHT));

        // Update count text
        std::ostringstream countStream;
        countStream << "x " << remainingUses;
        m_abilityCountText.setString(countStream.str());
    }

    void Hud::updateLives(int lives)
    {
        m_currentLives = std::clamp(lives, 0, static_cast<int>(m_lifeIcons.size()));
    }

    void Hud::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Draw growth section
        target.draw(m_growthLabel, states);
        target.draw(m_growthBarBackground, states);
        target.draw(m_growthBarFill, states);
        target.draw(m_growthBarOutline, states);

        // Draw score section
        target.draw(m_scoreLabel, states);
        target.draw(m_scoreValue, states);

        // Draw ability section
        target.draw(m_abilityLabel, states);
        target.draw(m_abilityBarBackground, states);
        target.draw(m_abilityBarFill, states);
        target.draw(m_abilityBarOutline, states);
        target.draw(m_abilityFishIcon, states);
        target.draw(m_abilityCountText, states);

        // Draw life icons (only the ones for current lives)
        for (int i = 0; i < m_currentLives; ++i)
        {
            target.draw(m_lifeIcons[i], states);
        }
    }
}