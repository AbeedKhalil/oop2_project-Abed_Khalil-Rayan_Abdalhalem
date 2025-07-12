#pragma once
#include "State.h"
#include <string>

namespace FishGame {
class PlayerNameState;
template<> struct is_state<PlayerNameState> : std::true_type {};

class PlayerNameState : public State {
public:
    explicit PlayerNameState(Game& game);
    ~PlayerNameState() override = default;

    void handleEvent(const sf::Event& event) override;
    bool update(sf::Time dt) override;
    void render() override;
    void onActivate() override;

private:
    std::string m_input;
    sf::Text m_prompt;
    sf::Text m_inputText;
    sf::Sprite m_backgroundSprite;
    sf::Sprite m_overlaySprite;
};
}
