#pragma once

#include "State.h"
#include "StateUtils.h"
#include "GameConstants.h"
#include "Managers/SpriteManager.h"
#include <unordered_map>
#include <vector>

namespace FishGame {
class StageIntroState;

class StageSummaryState;
template<> struct is_state<StageSummaryState> : std::true_type {};

struct StageSummaryConfig {
    int nextLevel = 1;
    int levelScore = 0;
    std::unordered_map<TextureID, int> counts;
    static StageSummaryConfig& getInstance() {
        static StageSummaryConfig instance;
        return instance;
    }
};

class StageSummaryState : public State {
public:
    explicit StageSummaryState(Game& game);
    ~StageSummaryState() override = default;

    static void configure(int nextLevel, int levelScore,
                          const std::unordered_map<TextureID, int>& counts);

    void handleEvent(const sf::Event& event) override;
    bool update(sf::Time deltaTime) override;
    void render() override;
    void onActivate() override;

private:
    struct Item {
        sf::Sprite sprite;
        sf::Text text;
    };

    void setupItems();
    void exitState();

    sf::Sprite m_overlaySprite;
    sf::Text m_scoreText;
    sf::Sprite m_nextButtonSprite;
    sf::Text m_nextText;
    bool m_buttonHovered{false};
    std::vector<Item> m_items;
};
} // namespace FishGame
