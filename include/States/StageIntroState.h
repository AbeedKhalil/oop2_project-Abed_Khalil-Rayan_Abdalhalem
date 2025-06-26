#pragma once

#include "GameConstants.h"
#include "State.h"
#include "StateUtils.h"
#include "Managers/SpriteManager.h"
#include <string>
#include <vector>

namespace FishGame {
enum class TextureID;
class StageIntroState;
template <> struct is_state<StageIntroState> : std::true_type {};

struct StageIntroConfig {
  int level = 1;
  bool pushPlay = true;
  static StageIntroConfig &getInstance() {
    static StageIntroConfig instance;
    return instance;
  }
};

class StageIntroState : public State {
public:
  explicit StageIntroState(Game &game);
  ~StageIntroState() override = default;

  static void configure(int level, bool pushPlay);

  void handleEvent(const sf::Event &event) override;
  bool update(sf::Time deltaTime) override;
  void render() override;
  void onActivate() override;

private:
  struct Item {
    sf::Sprite sprite;
    sf::Text text;
    TextureID tex{TextureID::Background1};
  };

  void setupItems();
  void exitState();

  sf::Sprite m_backgroundSprite;
  sf::Sprite m_overlaySprite;
  std::vector<Item> m_items;
  sf::Time m_elapsed;
  int m_level;
  bool m_pushPlay;
  static constexpr float DISPLAY_TIME = 3.0f;
};
} // namespace FishGame
