#pragma once

#include <SFML/Graphics.hpp>

namespace FishGame
{
    namespace Constants
    {
        // Window Settings
        constexpr unsigned int WINDOW_WIDTH = 1920;
        constexpr unsigned int WINDOW_HEIGHT = 1080;
        constexpr unsigned int FRAMERATE_LIMIT = 60;

        // Player Settings
        constexpr float PLAYER_BASE_SPEED = 400.0f;
        constexpr float PLAYER_ACCELERATION = 10.0f;
        constexpr float PLAYER_DECELERATION = 8.0f;
        constexpr float PLAYER_MAX_SPEED = 600.0f;
        constexpr float PLAYER_BASE_RADIUS = 20.0f;
        constexpr float PLAYER_GROWTH_FACTOR = 1.5f;

        // Game Rules
        constexpr int INITIAL_LIVES = 3;
        constexpr int STAGE_1_THRESHOLD = 0;
        constexpr int STAGE_2_THRESHOLD = 33;
        constexpr int STAGE_3_THRESHOLD = 66;
        constexpr int MAX_SCORE = 150;  // Updated from 100
        constexpr int LEVEL_COMPLETE_SCORE = 150;  // Updated from 100

        // Fish Settings
        constexpr float SMALL_FISH_RADIUS = 15.0f;
        constexpr float MEDIUM_FISH_RADIUS = 25.0f;
        constexpr float LARGE_FISH_RADIUS = 35.0f;

        constexpr float SMALL_FISH_SPEED = 150.0f;
        constexpr float MEDIUM_FISH_SPEED = 120.0f;
        constexpr float LARGE_FISH_SPEED = 90.0f;

        // Point values now depend on level - moved to method

        // AI Settings
        constexpr float AI_DETECTION_RANGE = 80.0f;
        constexpr float AI_FLEE_RANGE = 65.0f;  // New flee range
        constexpr float SPAWN_MARGIN = 50.0f;

        // Visual Settings
        constexpr float HUD_MARGIN = 20.0f;
        constexpr unsigned int HUD_FONT_SIZE = 24;
        constexpr unsigned int MESSAGE_FONT_SIZE = 48;

        // Progress Bar Settings
        constexpr float PROGRESS_BAR_WIDTH = 200.0f;
        constexpr float PROGRESS_BAR_HEIGHT = 20.0f;
        constexpr float PROGRESS_BAR_OUTLINE = 2.0f;

        // Timing
        const sf::Time INVULNERABILITY_DURATION = sf::seconds(2.0f);
        const sf::Time LEVEL_TRANSITION_DURATION = sf::seconds(3.0f);
        const sf::Time RESPAWN_MESSAGE_DURATION = sf::seconds(2.0f);
        const sf::Time SCORE_FLASH_DURATION = sf::seconds(0.5f);

        // Colors
        const sf::Color OCEAN_BLUE(0, 100, 150);
        const sf::Color PLAYER_COLOR = sf::Color::Yellow;
        const sf::Color PLAYER_OUTLINE(255, 200, 0);
        const sf::Color SMALL_FISH_COLOR = sf::Color::Green;
        const sf::Color SMALL_FISH_OUTLINE(0, 100, 0);
        const sf::Color MEDIUM_FISH_COLOR = sf::Color::Blue;
        const sf::Color MEDIUM_FISH_OUTLINE(0, 0, 100);
        const sf::Color LARGE_FISH_COLOR = sf::Color::Red;
        const sf::Color LARGE_FISH_OUTLINE(100, 0, 0);
        const sf::Color PROGRESS_BAR_FILL(0, 255, 0);
        const sf::Color PROGRESS_BAR_BACKGROUND(50, 50, 50);
        const sf::Color PROGRESS_BAR_OUTLINE_COLOR(255, 255, 255);
    }
}