#include "GameOptionsState.h"
#include "Game.h"

namespace {
using FishGame::TextureID;
sf::IntRect firstFrameRect(TextureID id) {
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
GameOptionsState::GameOptionsState(Game &game)
    : State(game), m_titleText(), m_instructionText(), m_gameDescriptionText(),
      m_controlsText(), m_musicVolumeText(), m_soundVolumeText(),
      m_overlaySprite(), m_backButtonSprite(), m_nextButtonSprite(), m_backText(),
      m_nextText(), m_background() {}

void GameOptionsState::onActivate() {
  auto &window = getGame().getWindow();
  auto &font = getGame().getFonts().get(Fonts::Main);
  auto &manager = getGame().getSpriteManager();

  m_musicVolume = getGame().getMusicPlayer().getVolume();
  m_soundVolume = getGame().getSoundPlayer().getVolume();

  m_background.setSize(sf::Vector2f(window.getSize()));
  m_background.setFillColor(Constants::OVERLAY_COLOR);

  m_overlaySprite.setTexture(manager.getTexture(TextureID::StageIntro));
  auto size = m_overlaySprite.getTexture()->getSize();
  m_overlaySprite.setScale(static_cast<float>(window.getSize().x) / size.x,
                           static_cast<float>(window.getSize().y) / size.y);

  m_titleText.setFont(font);
  m_titleText.setString("OPTIONS");
  m_titleText.setCharacterSize(72);
  m_titleText.setFillColor(sf::Color::White);
  auto bounds = m_titleText.getLocalBounds();
  m_titleText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
  float winWidth = static_cast<float>(window.getSize().x);
  float winHeight = static_cast<float>(window.getSize().y);
  m_titleText.setPosition(winWidth / 2.f, 180.f);

  m_gameDescriptionText.setFont(font);
  m_gameDescriptionText.setString(
      "Eat smaller fish to grow and avoid larger predators.");
  m_gameDescriptionText.setCharacterSize(30);
  m_gameDescriptionText.setFillColor(sf::Color::White);
  bounds = m_gameDescriptionText.getLocalBounds();
  m_gameDescriptionText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
  m_gameDescriptionText.setPosition(winWidth / 2.f, 250.f);

  m_controlsText.setFont(font);
  m_controlsText.setString("Move with Arrow Keys, Space to dash");
  m_controlsText.setCharacterSize(30);
  m_controlsText.setFillColor(sf::Color::White);
  bounds = m_controlsText.getLocalBounds();
  m_controlsText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
  m_controlsText.setPosition(winWidth / 2.f, 290.f);

  m_instructionText.setFont(font);
  m_instructionText.setString("Use Arrows to change volume");
  m_instructionText.setCharacterSize(36);
  m_instructionText.setFillColor(sf::Color::White);
  bounds = m_instructionText.getLocalBounds();
  m_instructionText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
  m_instructionText.setPosition(winWidth / 2.f, winHeight / 2.f + 200.f);

  m_musicVolumeText.setFont(font);
  m_musicVolumeText.setCharacterSize(48);
  m_musicVolumeText.setFillColor(sf::Color::White);

  m_soundVolumeText.setFont(font);
  m_soundVolumeText.setCharacterSize(48);
  m_soundVolumeText.setFillColor(sf::Color::White);

  m_backButtonSprite.setTexture(manager.getTexture(TextureID::Button));
  auto b = m_backButtonSprite.getLocalBounds();
  m_backButtonSprite.setOrigin(b.width / 2.f, b.height / 2.f);
  m_backButtonSprite.setScale(Constants::MENU_BUTTON_SCALE,
                              Constants::MENU_BUTTON_SCALE);
  m_backButtonSprite.setPosition(winWidth / 2.f - 200.f, winHeight - 150.f);

  m_backText.setFont(font);
  m_backText.setString("BACK");
  m_backText.setCharacterSize(36);
  auto bb = m_backText.getLocalBounds();
  m_backText.setOrigin(bb.width / 2.f, bb.height / 2.f + 10.f);
  m_backText.setPosition(m_backButtonSprite.getPosition());
  m_backText.setFillColor(sf::Color(0, 16, 112));

  m_nextButtonSprite.setTexture(manager.getTexture(TextureID::Button));
  auto nbounds = m_nextButtonSprite.getLocalBounds();
  m_nextButtonSprite.setOrigin(nbounds.width / 2.f, nbounds.height / 2.f);
  m_nextButtonSprite.setScale(Constants::MENU_BUTTON_SCALE,
                              Constants::MENU_BUTTON_SCALE);
  m_nextButtonSprite.setPosition(winWidth / 2.f + 200.f, winHeight - 150.f);

  m_nextText.setFont(font);
  m_nextText.setString("NEXT");
  m_nextText.setCharacterSize(36);
  auto nt = m_nextText.getLocalBounds();
  m_nextText.setOrigin(nt.width / 2.f, nt.height / 2.f + 10.f);
  m_nextText.setPosition(m_nextButtonSprite.getPosition());
  m_nextText.setFillColor(sf::Color(0, 16, 112));

  m_backButtonHovered = false;
  m_nextButtonHovered = false;

  setupInfoItems();
  updateCurrentInfo();

  updateVolumeTexts();
}

void GameOptionsState::updateVolumeTexts() {
  auto &window = getGame().getWindow();
  m_musicVolumeText.setString("Music Volume: " +
                              std::to_string(static_cast<int>(m_musicVolume)));
  m_soundVolumeText.setString("Sound Volume: " +
                              std::to_string(static_cast<int>(m_soundVolume)));

  auto mb = m_musicVolumeText.getLocalBounds();
  m_musicVolumeText.setOrigin(mb.width / 2.f, mb.height / 2.f);
  m_musicVolumeText.setPosition(static_cast<float>(window.getSize().x) / 2.f,
                                static_cast<float>(window.getSize().y) / 2.f -
                                    40.f);

  auto sb = m_soundVolumeText.getLocalBounds();
  m_soundVolumeText.setOrigin(sb.width / 2.f, sb.height / 2.f);
  m_soundVolumeText.setPosition(static_cast<float>(window.getSize().x) / 2.f,
                                static_cast<float>(window.getSize().y) / 2.f +
                                    40.f);
}

void GameOptionsState::handleEvent(const sf::Event &event) {
  auto &music = getGame().getMusicPlayer();
  auto &sounds = getGame().getSoundPlayer();

  if (event.type == sf::Event::KeyPressed) {
    switch (event.key.code) {
    case sf::Keyboard::Escape:
      deferAction([this]() { requestStackPop(); });
      break;
    case sf::Keyboard::Up:
      m_musicVolume = std::min(100.f, m_musicVolume + 5.f);
      music.setVolume(m_musicVolume);
      updateVolumeTexts();
      break;
    case sf::Keyboard::Down:
      m_musicVolume = std::max(0.f, m_musicVolume - 5.f);
      music.setVolume(m_musicVolume);
      updateVolumeTexts();
      break;
    case sf::Keyboard::Right:
      m_soundVolume = std::min(100.f, m_soundVolume + 5.f);
      sounds.setVolume(m_soundVolume);
      updateVolumeTexts();
      break;
    case sf::Keyboard::Left:
      m_soundVolume = std::max(0.f, m_soundVolume - 5.f);
      sounds.setVolume(m_soundVolume);
      updateVolumeTexts();
      break;
    default:
      break;
    }
  } else if (event.type == sf::Event::MouseMoved) {
    sf::Vector2f pos(static_cast<float>(event.mouseMove.x),
                     static_cast<float>(event.mouseMove.y));
    bool hoverBack = m_backButtonSprite.getGlobalBounds().contains(pos);
    if (hoverBack != m_backButtonHovered) {
      m_backButtonHovered = hoverBack;
      auto &manager = getGame().getSpriteManager();
      m_backButtonSprite.setTexture(
          manager.getTexture(hoverBack ? TextureID::ButtonHover : TextureID::Button));
    }
    bool hoverNext = m_nextButtonSprite.getGlobalBounds().contains(pos);
    if (hoverNext != m_nextButtonHovered) {
      m_nextButtonHovered = hoverNext;
      auto &manager = getGame().getSpriteManager();
      m_nextButtonSprite.setTexture(
          manager.getTexture(hoverNext ? TextureID::ButtonHover : TextureID::Button));
    }
  } else if (event.type == sf::Event::MouseButtonPressed &&
             event.mouseButton.button == sf::Mouse::Left) {
    sf::Vector2f pos(static_cast<float>(event.mouseButton.x),
                     static_cast<float>(event.mouseButton.y));
    if (m_backButtonSprite.getGlobalBounds().contains(pos)) {
      deferAction([this]() { requestStackPop(); });
    } else if (m_nextButtonSprite.getGlobalBounds().contains(pos)) {
      m_currentIndex = (m_currentIndex + 1) % m_infoItems.size();
      updateCurrentInfo();
    }
  }
}

bool GameOptionsState::update(sf::Time) {
  processDeferredActions();
  return false;
}

void GameOptionsState::render() {
  auto &window = getGame().getWindow();
  window.draw(m_background);
  window.draw(m_overlaySprite);
  window.draw(m_titleText);
  window.draw(m_gameDescriptionText);
  window.draw(m_controlsText);
  // Display volume controls only on the first page
  if (m_currentIndex == 0) {
    window.draw(m_musicVolumeText);
    window.draw(m_soundVolumeText);
    window.draw(m_instructionText);
  }
  if (!m_infoItems.empty()) {
    window.draw(m_infoItems[m_currentIndex].sprite);
    window.draw(m_infoItems[m_currentIndex].text);
  }
  window.draw(m_backButtonSprite);
  window.draw(m_backText);
  window.draw(m_nextButtonSprite);
  window.draw(m_nextText);
}

void GameOptionsState::setupInfoItems() {
  m_infoItems.clear();
  auto &manager = getGame().getSpriteManager();
  auto &font = getGame().getFonts().get(Fonts::Main);

  auto add = [&](TextureID tex, const std::string &str) {
    InfoItem item;
    item.tex = tex;
    item.sprite.setTexture(manager.getTexture(tex));
    item.text.setFont(font);
    item.text.setCharacterSize(32);
    item.text.setFillColor(sf::Color::White);
    item.text.setString(str);
    m_infoItems.push_back(std::move(item));
  };

  add(TextureID::SmallFish, "Small fish - easy prey");
  add(TextureID::MediumFish, "Medium fish - worth more points");
  add(TextureID::LargeFish, "Large fish - avoid until bigger");
  add(TextureID::Angelfish, "Angelfish - bonus points");
  add(TextureID::PoisonFish, "Poison fish - reverses controls");
  add(TextureID::Pufferfish, "Pufferfish - inflates when threatened");
  add(TextureID::Barracuda, "Barracuda - fast predator");
  add(TextureID::Bomb, "Bomb - explodes on contact");
  add(TextureID::Jellyfish, "Jellyfish - stuns on touch");
}

void GameOptionsState::updateCurrentInfo() {
  if (m_infoItems.empty())
    return;

  auto &window = getGame().getWindow();
  auto &item = m_infoItems[m_currentIndex];

  sf::IntRect rect = firstFrameRect(item.tex);
  if (rect.width > 0)
    item.sprite.setTextureRect(rect);

  auto bounds = item.sprite.getLocalBounds();
  item.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
  // make the example sprite easier to see in the options menu
  item.sprite.setScale(0.8f, 0.8f);
  item.sprite.setPosition(static_cast<float>(window.getSize().x) / 2.f,
                          static_cast<float>(window.getSize().y) / 2.f + 60.f);

  auto tb = item.text.getLocalBounds();
  item.text.setOrigin(tb.width / 2.f, tb.height / 2.f);
  item.text.setPosition(item.sprite.getPosition().x,
                        item.sprite.getPosition().y + bounds.height / 2.f + 40.f);
}
} // namespace FishGame
