#include "Levels/LevelTable.h"

namespace FishGame {

const std::unordered_map<int, LevelDef> LEVELS = {
    {1, { { {"Fish",3} }, {"Life","Star"}, "Survive" }},
    {2, { { {"Fish",3},{"Oyster",1} }, {"Speed","x2","Frenzy","Freeze"}, "Survive"}},
    {3, { { {"Fish",3},{"PoisonFish",1},{"AngelFish",1} }, {}, "Survive"}},
    {100, { { {"Oyster",3} }, {"Add-Time"}, "Eat oysters"}},
    {4, { { {"Fish",3},{"Jellyfish",1},{"Pufferfish",1} }, {}, "Survive"}},
    {5, { { {"Fish",3},{"Barracuda",1} }, {}, "Survive"}},
    {6, { { {"Fish",3},{"Bomb",2} }, {}, "Survive"}},
    {200, { { {"SmallFish",5} }, {}, "Eat small fish & avoid bombs"}},
    {7, { { {"All",10} }, {}, "Final challenge"}}
};

} // namespace FishGame

