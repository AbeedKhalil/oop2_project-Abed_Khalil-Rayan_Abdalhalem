#pragma once
#include "State.h"
#include "GameConstants.h"
#include "Utils/HighScoreIO.h"
#include <vector>
#include <unordered_set>

namespace FishGame {
class HighScoresState;
template<> struct is_state<HighScoresState> : std::true_type {};

struct HighScoreEntryHash {
    std::size_t operator()(const HighScoreEntry& e) const noexcept {
        return std::hash<std::string>{}(e.name);
    }
};

struct HighScoreEntryEqual {
    bool operator()(const HighScoreEntry& a, const HighScoreEntry& b) const noexcept {
        return a.name == b.name;
    }
};

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
    std::unordered_set<HighScoreEntry, HighScoreEntryHash, HighScoreEntryEqual> m_scoreSet;
    std::vector<HighScoreEntry> m_scores;
    sf::Text m_titleText;
    std::vector<sf::Text> m_scoreTexts;
    sf::Sprite m_backButton;
    sf::Text m_backText;
    bool m_hover{false};
    sf::Sprite m_backgroundSprite;
    sf::Sprite m_overlaySprite;
};
}
