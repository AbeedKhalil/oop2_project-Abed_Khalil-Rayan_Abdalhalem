#pragma once

#include <SFML/Graphics.hpp>

namespace FishGame
{
    namespace Constants
    {
        // ==================== Window Settings ====================
        constexpr unsigned int WINDOW_WIDTH = 1920;
        constexpr unsigned int WINDOW_HEIGHT = 1080;
        constexpr unsigned int FRAMERATE_LIMIT = 60;

        // ==================== Mathematical Constants ====================
        constexpr float PI = 3.14159265359f;
        constexpr float DEG_TO_RAD = PI / 180.0f;
        constexpr float RAD_TO_DEG = 180.0f / PI;

        // ==================== Player Settings ====================
        constexpr float PLAYER_BASE_SPEED = 400.0f;
        constexpr float PLAYER_ACCELERATION = 10.0f;
        constexpr float PLAYER_DECELERATION = 8.0f;
        constexpr float PLAYER_MAX_SPEED = 600.0f;
        constexpr float PLAYER_BASE_RADIUS = 20.0f;
        constexpr float PLAYER_GROWTH_FACTOR = 1.5f;
        constexpr float SPEED_BOOST_MULTIPLIER = 1.5f;

        // ==================== Points System Values ====================
        constexpr int SMALL_FISH_POINTS = 4;
        constexpr int MEDIUM_FISH_POINTS = 10;
        constexpr int LARGE_FISH_POINTS = 20;
        constexpr int WHITE_OYSTER_POINTS = 14;
        constexpr int BLACK_OYSTER_POINTS = 30;
        constexpr int BARRACUDA_POINTS = 26;
        constexpr int ANGELFISH_POINTS = 100;
        constexpr int PUFFERFISH_POINTS = 7;
        constexpr int PUFFERFISH_SCORE_PENALTY = 20;

        // ==================== Game Rules - Points Thresholds ====================
        constexpr int INITIAL_LIVES = 3;
        constexpr int POINTS_FOR_STAGE_2 = 100;
        constexpr int POINTS_FOR_STAGE_3 = 200;
        constexpr int POINTS_TO_WIN = 400;
        constexpr int MAX_STAGES = 3;

        // ==================== Fish Settings ====================
        constexpr float SMALL_FISH_RADIUS = 15.0f;
        constexpr float MEDIUM_FISH_RADIUS = 25.0f;
        constexpr float LARGE_FISH_RADIUS = 35.0f;

        constexpr float SMALL_FISH_SPEED = 150.0f;
        constexpr float MEDIUM_FISH_SPEED = 120.0f;
        constexpr float LARGE_FISH_SPEED = 90.0f;

        // ==================== AI Settings ====================
        constexpr float AI_DETECTION_RANGE = 80.0f;
        constexpr float AI_FLEE_RANGE = 65.0f;
        constexpr float SPAWN_MARGIN = 50.0f;

        // ==================== Spawn Rates ====================
        constexpr float BARRACUDA_SPAWN_RATE = 0.05f;
        constexpr float PUFFERFISH_SPAWN_RATE = 0.1f;
        constexpr float ANGELFISH_SPAWN_RATE = 0.05f;
        constexpr float SCHOOL_SPAWN_CHANCE = 0.05f;
        constexpr float MAX_SCHOOL_SPAWN_CHANCE = 0.10f;
        constexpr float POISONFISH_SPAWN_RATE = 0.15f;

        // ==================== Visual Settings ====================
        constexpr float HUD_MARGIN = 20.0f;
        constexpr unsigned int HUD_FONT_SIZE = 24;
        constexpr unsigned int HUD_SMALL_FONT_SIZE = 20;
        constexpr unsigned int MESSAGE_FONT_SIZE = 48;
        constexpr float HUD_LINE_SPACING = 30.0f;
        constexpr float POWERUP_TEXT_X_OFFSET = 300.0f;
        constexpr float FPS_TEXT_X_OFFSET = 100.0f;

        // ==================== System UI Positions ====================
        constexpr float FRENZY_Y_POSITION = 100.0f;

        // ==================== Menu Constants ====================
        constexpr const char* GAME_TITLE = "FEEDING FRENZY";
        constexpr float TITLE_Y_POSITION = 200.0f;

        constexpr float MENU_START_Y = 500.0f;
        constexpr float MENU_ITEM_SPACING = 180.0f;
        // Scale factor applied to menu button sprites
        constexpr float MENU_BUTTON_SCALE = 0.875f;

        // ==================== Animation Constants ====================
        constexpr float MENU_PULSE_SPEED = 2.0f;
        constexpr float MENU_PULSE_AMPLITUDE = 0.1f;
        constexpr float MENU_FADE_SPEED = 500.0f;

        // ==================== Timing ====================
        const sf::Time FPS_UPDATE_INTERVAL = sf::seconds(1.0f);
        const sf::Time SCHOOL_EXTRACT_INTERVAL = sf::seconds(0.1f);
        const sf::Time WIN_SEQUENCE_DURATION = sf::seconds(5.0f);

        // ==================== Particle Effects ====================
        constexpr int DEFAULT_PARTICLE_COUNT = 8;
        constexpr int ANGELFISH_PARTICLE_COUNT = 12;
        constexpr float PARTICLE_RADIUS = 3.0f;
        constexpr float MIN_PARTICLE_SPEED = 50.0f;
        constexpr float MAX_PARTICLE_SPEED = 150.0f;
        constexpr float PARTICLE_LIFETIME = 1.0f;
        constexpr float PARTICLE_INITIAL_ALPHA = 255.0f;
        constexpr float PARTICLE_FADE_RATE = 255.0f;

        // ==================== Container Limits ====================
        constexpr int MAX_ENTITIES = 100;
        constexpr int MAX_BONUS_ITEMS = 20;
        constexpr int MAX_PARTICLES = 200;

        // ==================== Difficulty ====================
        constexpr float DIFFICULTY_INCREMENT = 0.1f;

        // ==================== PowerUp Settings ====================
        constexpr float FREEZE_POWERUP_DURATION = 5.0f;
        constexpr float SPEEDBOOST_POWERUP_DURATION = 8.0f;
        constexpr float EXTRA_LIFE_HEARTBEAT_SPEED = 3.0f;
        constexpr float SCORE_DOUBLER_POWERUP_DURATION = 10.0f;
        constexpr float SCORE_DOUBLER_MULTIPLIER = 2.0f;

        // ==================== Colors - General ====================
        const sf::Color OCEAN_BLUE(0, 100, 150);

        // ==================== Colors - Fish ====================
        const sf::Color SMALL_FISH_COLOR = sf::Color::Green;
        const sf::Color SMALL_FISH_OUTLINE(0, 100, 0);
        const sf::Color MEDIUM_FISH_COLOR = sf::Color::Blue;
        const sf::Color MEDIUM_FISH_OUTLINE(0, 0, 100);
        const sf::Color LARGE_FISH_COLOR = sf::Color::Red;
        const sf::Color LARGE_FISH_OUTLINE(100, 0, 0);

        // ==================== Colors - UI ====================
        const sf::Color PROGRESS_BAR_FILL(0, 255, 0);
        const sf::Color PROGRESS_BAR_BACKGROUND(50, 50, 50);
        const sf::Color PROGRESS_BAR_OUTLINE_COLOR(255, 255, 255);
        const sf::Color HUD_TEXT_COLOR = sf::Color::White;
        const sf::Color OVERLAY_COLOR(0, 0, 0, 128);

        // ==================== Colors - Messages ====================
        const sf::Color MESSAGE_COLOR = sf::Color::Yellow;
        const sf::Color MESSAGE_OUTLINE_COLOR = sf::Color::Black;
        constexpr float MESSAGE_OUTLINE_THICKNESS = 2.0f;

        // ==================== Colors - Particles ====================
        const sf::Color DEATH_PARTICLE_COLOR = sf::Color::Red;
        const sf::Color EAT_PARTICLE_COLOR = sf::Color::Green;
        const sf::Color DAMAGE_PARTICLE_COLOR = sf::Color::Red;
        const sf::Color RESPAWN_PARTICLE_COLOR = sf::Color::Cyan;
        const sf::Color BONUS_PARTICLE_COLOR = sf::Color::Yellow;
        const sf::Color TAILBITE_PARTICLE_COLOR = sf::Color::Magenta;
        const sf::Color OYSTER_IMPACT_COLOR(100, 100, 100);
        const sf::Color ANGELFISH_PARTICLE_COLOR = sf::Color::Cyan;
        const sf::Color PUFFERFISH_IMPACT_COLOR = sf::Color::Yellow;
        const sf::Color BLACK_PEARL_COLOR = sf::Color::Magenta;
        const sf::Color WHITE_PEARL_COLOR = sf::Color::White;

        // ==================== Colors - PowerUps ====================
        const sf::Color SCORE_DOUBLER_COLOR = sf::Color::Yellow;
        const sf::Color FRENZY_STARTER_COLOR = sf::Color::Magenta;
        const sf::Color SPEED_BOOST_COLOR = sf::Color::Cyan;
    }
}
