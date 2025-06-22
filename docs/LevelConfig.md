# Level Configuration

Levels describe which enemies and power-ups appear when a stage begins. The
`Level` template stores collections of `Entity` objects and can be loaded using a
simple `LevelConfig` structure:

```cpp
struct LevelConfig {
    std::vector<sf::Vector2f> enemyPositions;   // where enemies spawn
    std::vector<sf::Vector2f> powerUpPositions; // where power-ups appear
};
```

```cpp
Level<SmallFish, ExtraLifePowerUp> level;
LevelConfig config;
config.enemyPositions.push_back({100.f, 100.f});
config.powerUpPositions.push_back({200.f, 200.f});
level.load(config);
```

Each level is complete when all enemies and power-ups have been cleared.
