#include "StageIntroState.h"
#include "Game.h"
#include <string>

namespace {
FishGame::TextureID backgroundForLevel(int level) {
  static const FishGame::TextureID backgrounds[] = {
      FishGame::TextureID::Background1,
      FishGame::TextureID::Background2,
      FishGame::TextureID::Background3,
      FishGame::TextureID::Background4,
      FishGame::TextureID::Background5};
  int index = ((level - 1) / 2) % 5;
  return backgrounds[index];
}

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
  case TextureID::PearlOysterClosed:
    return {1, 1, 101, 101};
  case TextureID::PearlOysterOpen:
    return {1 + 4 * 101, 1, 101, 101};
  case TextureID::Bomb:
    return {1, 1, 69, 69};
  case TextureID::Pufferfish:
    return {5, 5, 187, 131};
  case TextureID::PufferfishInflated:
    return {5, 136, 186, 169};
  case TextureID::Jellyfish:
    return {1, 1, 75, 197};
  case TextureID::Barracuda:
    return {1, 1, 270, 122};
  default:
    return {};
  }
}
} // namespace

namespace FishGame {
StageIntroState::StageIntroState(Game &game)
    : State(game), m_backgroundSprite(), m_overlaySprite(), m_items(), m_elapsed(sf::Time::Zero),
      m_level(StageIntroConfig::getInstance().level),
      m_pushPlay(StageIntroConfig::getInstance().pushNext),
      m_nextState(StageIntroConfig::getInstance().nextState) {}

void StageIntroState::configure(int level, bool pushNext, StateID nextState) {
  auto &cfg = StageIntroConfig::getInstance();
  cfg.level = level;
  cfg.pushNext = pushNext;
  cfg.nextState = nextState;
}

void StageIntroState::onActivate() {
  auto &manager = getGame().getSpriteManager();
  auto &window = getGame().getWindow();
  m_backgroundSprite.setTexture(
      manager.getTexture(backgroundForLevel(m_level)));
  auto texSize = m_backgroundSprite.getTexture()->getSize();
  m_backgroundSprite.setScale(
      static_cast<float>(window.getSize().x) / texSize.x,
      static_cast<float>(window.getSize().y) / texSize.y);

  m_overlaySprite.setTexture(manager.getTexture(TextureID::StageIntro));
  auto overlaySize = m_overlaySprite.getTexture()->getSize();
  m_overlaySprite.setScale(
      static_cast<float>(window.getSize().x) / overlaySize.x,
      static_cast<float>(window.getSize().y) / overlaySize.y);

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
  case 0:
    add(TextureID::Bomb, "Avoid bombs!");
    add(TextureID::SmallFish, "Eat small fish for points");
    add(TextureID::Starfish, "Collect starfish for points");
    add(TextureID::PowerUpAddTime, "Grab time power-ups to extend time");
    break;
  case 1:
    add(TextureID::SmallFish, "Eat small fish to grow");
    add(TextureID::MediumFish, "Eat Medium fish to become the king of the stage!");
    add(TextureID::LargeFish, "Eat Large fish to Win the level");
    add(TextureID::Starfish, "Collect starfish for points");
    add(TextureID::PowerUpExtraLife, "Extra life may appear");
    break;
  case 2:
    add(TextureID::PearlOysterClosed, "Oyster closed - stay away");
    add(TextureID::PearlOysterOpen, "Oyster open - collect pearls");
    add(TextureID::WhitePearl, "White pearl worth 100 points");
    add(TextureID::BlackPearl, "Black pearl worth 500 points");
    add(TextureID::PowerUpSpeedBoost, "Grab speed power-up to become faster");
    break;
  case 3:
    add(TextureID::PoisonFish, "Avoid poison fish!");
    add(TextureID::Angelfish, "Eat angelfish to grow to next staage");
    break;
  case 4:
    add(TextureID::Pufferfish, "Pufferfish inflates when threatened");
    add(TextureID::PufferfishInflated, "Avoid it while puffed!");
    add(TextureID::Jellyfish, "Jellyfish will stun you");
    break;
  case 5:
    add(TextureID::Barracuda, "  Barracuda is fast and dangerous");
    break;
  case 6:
      add(TextureID::Bomb, "Avoid bombs!");
      break;
  default:
      add(TextureID::SmallFish, "Eat small fish to grow");
      add(TextureID::MediumFish, "Eat Medium fish to become the king of the stage!");
      add(TextureID::LargeFish, "Eat Large fish to Win the level");
      add(TextureID::PoisonFish, "Avoid poison fish!");
      add(TextureID::Angelfish, "Eat angelfish to grow to next staage");
      add(TextureID::Pufferfish, "Pufferfish inflates when threatened");
      add(TextureID::PufferfishInflated, "Avoid it while puffed!");
      add(TextureID::Barracuda, "  Barracuda is fast and dangerous");
      add(TextureID::Jellyfish, "\nJellyfish will stun you");
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
      requestStackPush(m_nextState);
  });
}

void StageIntroState::render() {
  auto &window = getGame().getWindow();
  window.draw(m_backgroundSprite);
  window.draw(m_overlaySprite);
  for (auto &item : m_items) {
    window.draw(item.sprite);
    window.draw(item.text);
  }
}
} // namespace FishGame
