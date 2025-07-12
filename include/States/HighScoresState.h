#pragma once
#include "State.h"
#include "GameConstants.h"
#include "Utils/HighScoreIO.h"
#include <vector>

namespace FishGame {
class HighScoresState;
template<> struct is_state<HighScoresState> : std::true_type {};

class HighScoresState : public State {
public:
    explicit HighScoresState(Game& game);
    ~HighScoresState() override = default;

    void handleEvent(const sf::Event& event) override;
    bool update(sf::Time dt) override;
    void render() override;
    void onActivate() override;

private:
    void loadScores();
    std::vector<HighScoreEntry> m_scores;
    sf::Text m_titleText;
    std::vector<sf::Text> m_scoreTexts;
    sf::Sprite m_backButton;
    bool m_hover{false};
};
}
