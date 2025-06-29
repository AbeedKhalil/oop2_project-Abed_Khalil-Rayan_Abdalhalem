#pragma once

#include "GameConstants.h"
#include "State.h"

namespace FishGame {
class GameOptionsState;
template <> struct is_state<GameOptionsState> : std::true_type {};

class GameOptionsState : public State {
public:
  explicit GameOptionsState(Game &game);
  ~GameOptionsState() override = default;

  void handleEvent(const sf::Event &event) override;
  bool update(sf::Time deltaTime) override;
  void render() override;
  void onActivate() override;
  void updateVolumeTexts();

private:
  sf::Text m_titleText;
  sf::Text m_instructionText;
  sf::Text m_gameDescriptionText;
  sf::Text m_controlsText;
  sf::Text m_musicVolumeText;
  sf::Text m_soundVolumeText;
  sf::Sprite m_overlaySprite;
  sf::Sprite m_backButtonSprite;
  sf::Text m_backText;
  sf::RectangleShape m_background;
  bool m_buttonHovered{false};
  float m_musicVolume{100.f};
  float m_soundVolume{100.f};
};
} // namespace FishGame
