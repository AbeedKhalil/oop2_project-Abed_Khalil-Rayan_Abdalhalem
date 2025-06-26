#include "StageIntroState.h"
#include "Game.h"
#include <string>

namespace {
sf::IntRect firstFrameRect(FishGame::TextureID id) {
  using namespace FishGame;
  switch (id) {
  case TextureID::SmallFish:
  case TextureID::PoisonFish:
  case TextureID::Angelfish:
    return {1, 1, 66, 44};
  case TextureID::MediumFish:
    return {1, 1, 172, 108};
  case TextureID::LargeFish:
    return {1, 1, 201, 148};
  default:
    return {};
  }
}
} // namespace

namespace FishGame {
StageIntroState::StageIntroState(Game &game)
    : State(game), m_backgroundSprite(), m_items(), m_elapsed(sf::Time::Zero),
      m_level(StageIntroConfig::getInstance().level),
      m_pushPlay(StageIntroConfig::getInstance().pushPlay) {}

void StageIntroState::configure(int level, bool pushPlay) {
  auto &cfg = StageIntroConfig::getInstance();
  cfg.level = level;
  cfg.pushPlay = pushPlay;
}

void StageIntroState::onActivate() {
  auto &manager = getGame().getSpriteManager();
  auto &window = getGame().getWindow();
  m_backgroundSprite.setTexture(manager.getTexture(TextureID::StageIntro));
  auto texSize = m_backgroundSprite.getTexture()->getSize();
  m_backgroundSprite.setScale(
      static_cast<float>(window.getSize().x) / texSize.x,
      static_cast<float>(window.getSize().y) / texSize.y);

  setupItems();
  m_elapsed = sf::Time::Zero;
}

void StageIntroState::setupItems() {
  m_items.clear();
  auto &manager = getGame().getSpriteManager();
  auto &font = getGame().getFonts().get(Fonts::Main);

  auto add = [&](TextureID tex, const std::string &str) {
    Item item;
    item.tex = tex;
    item.sprite.setTexture(manager.getTexture(tex));
    item.text.setFont(font);
    item.text.setString(str);
    item.text.setCharacterSize(28);
    m_items.push_back(std::move(item));
  };

  switch (m_level) {
  case 1:
    add(TextureID::SmallFish, "Eat small fish to grow");
    add(TextureID::MediumFish, "Eat Medium fish to become the king of the stage!");
    add(TextureID::LargeFish, "Eat Large fish to Win the level");
    add(TextureID::Starfish, "Collect starfish for points");
    add(TextureID::PowerUpExtraLife, "Extra life may appear");
    break;
  case 2:
    add(TextureID::PowerUpSpeedBoost, "Grab power-ups for bonuses");
    break;
  default:
    
    add(TextureID::PoisonFish, "Avoid poison fish!");
    break;
  }

  float startY = 300.f;
  float xLeft = 300.f;
  float xText = 400.f;
  for (std::size_t i = 0; i < m_items.size(); ++i) {
    auto &item = m_items[i];

    // Display only the first frame of the sprite sheet for fish
    sf::IntRect rect = firstFrameRect(item.tex);
    if (rect.width > 0)
      item.sprite.setTextureRect(rect);

    sf::FloatRect b = item.sprite.getLocalBounds();
    item.sprite.setOrigin(b.width / 2.f, b.height / 2.f);
    item.sprite.setPosition(xLeft, startY + i * 100.f);

    float scale = 0.75f;
    if (item.tex == TextureID::Starfish)
      scale = 0.02f;
    item.sprite.setScale(scale, scale);

    b = item.text.getLocalBounds();
    item.text.setOrigin(0.f, b.height / 2.f);
    item.text.setPosition(xText, startY + i * 100.f);
  }
}

void StageIntroState::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed ||
      event.type == sf::Event::MouseButtonPressed) {
    exitState();
  }
}

bool StageIntroState::update(sf::Time deltaTime) {
  processDeferredActions();
  return false;
}

void StageIntroState::exitState() {
  deferAction([this]() {
    requestStackPop();
    if (m_pushPlay)
      requestStackPush(StateID::Play);
  });
}

void StageIntroState::render() {
  auto &window = getGame().getWindow();
  window.draw(m_backgroundSprite);
  for (auto &item : m_items) {
    window.draw(item.sprite);
    window.draw(item.text);
  }
}
} // namespace FishGame
