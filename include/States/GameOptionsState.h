#pragma once

#include "GameConstants.h"
#include "SpriteManager.h"
#include "State.h"
#include <vector>

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
  struct InfoItem {
    sf::Sprite sprite;
    sf::Text text;
    TextureID tex{TextureID::SmallFish};
  };

  void setupInfoItems();
  void updateCurrentInfo();

  sf::Sprite m_backButtonSprite;
  sf::Sprite m_nextButtonSprite;
  sf::Text m_backText;
  sf::Text m_nextText;
  sf::RectangleShape m_background;
  bool m_backButtonHovered{false};
  bool m_nextButtonHovered{false};
  float m_musicVolume{100.f};
  float m_soundVolume{100.f};
  std::vector<InfoItem> m_infoItems;
  std::size_t m_currentIndex{0};
}; 
} // namespace FishGame
