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

        // Points System Values
        constexpr int SMALL_FISH_POINTS = 2;
        constexpr int MEDIUM_FISH_POINTS = 5;
        constexpr int LARGE_FISH_POINTS = 10;
        constexpr int WHITE_OYSTER_POINTS = 7;
        constexpr int BLACK_OYSTER_POINTS = 15;
        constexpr int BARRACUDA_POINTS = 13;
        constexpr int ANGELFISH_POINTS = 100;
        constexpr int PUFFERFISH_POINTS = 7;

        // Game Rules - Points Thresholds
        constexpr int INITIAL_LIVES = 3;
        constexpr int POINTS_FOR_STAGE_2 = 100;
        constexpr int POINTS_FOR_STAGE_3 = 200;
        constexpr int POINTS_TO_WIN = 400;
        constexpr int MAX_STAGES = 3;

        // Fish Settings
        constexpr float SMALL_FISH_RADIUS = 15.0f;
        constexpr float MEDIUM_FISH_RADIUS = 25.0f;
        constexpr float LARGE_FISH_RADIUS = 35.0f;

        constexpr float SMALL_FISH_SPEED = 150.0f;
        constexpr float MEDIUM_FISH_SPEED = 120.0f;
        constexpr float LARGE_FISH_SPEED = 90.0f;

        // AI Settings
        constexpr float AI_DETECTION_RANGE = 80.0f;
        constexpr float AI_FLEE_RANGE = 65.0f;
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