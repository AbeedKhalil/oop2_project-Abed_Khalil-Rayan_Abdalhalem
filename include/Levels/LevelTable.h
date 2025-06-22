#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace FishGame {

struct EntityInfo {
    std::string type;
    unsigned count;
};

struct LevelDef {
    std::vector<EntityInfo> enemies;
    std::vector<std::string> powerUps;
    std::string goal;
};

extern const std::unordered_map<int, LevelDef> LEVELS;

} // namespace FishGame

