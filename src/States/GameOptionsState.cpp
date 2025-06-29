#include "GameOptionsState.h"
#include "Game.h"

namespace FishGame {
GameOptionsState::GameOptionsState(Game &game)
    : State(game), m_titleText(), m_instructionText(), m_musicVolumeText(),
      m_soundVolumeText(), m_overlaySprite(), m_backButtonSprite(),
      m_backText(), m_background() {}

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
  m_backButtonSprite.setPosition(winWidth / 2.f, winHeight - 150.f);

  m_backText.setFont(font);
  m_backText.setString("BACK");
  m_backText.setCharacterSize(36);
  auto bb = m_backText.getLocalBounds();
  m_backText.setOrigin(bb.width / 2.f, bb.height / 2.f + 10.f);
  m_backText.setPosition(m_backButtonSprite.getPosition());
  m_backText.setFillColor(sf::Color(0, 16, 112));

  m_buttonHovered = false;

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
    bool hover = m_backButtonSprite.getGlobalBounds().contains(pos);
    if (hover != m_buttonHovered) {
      m_buttonHovered = hover;
      auto &manager = getGame().getSpriteManager();
      m_backButtonSprite.setTexture(manager.getTexture(
          hover ? TextureID::ButtonHover : TextureID::Button));
    }
  } else if (event.type == sf::Event::MouseButtonPressed &&
             event.mouseButton.button == sf::Mouse::Left) {
    sf::Vector2f pos(static_cast<float>(event.mouseButton.x),
                     static_cast<float>(event.mouseButton.y));
    if (m_backButtonSprite.getGlobalBounds().contains(pos)) {
      deferAction([this]() { requestStackPop(); });
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
  window.draw(m_musicVolumeText);
  window.draw(m_soundVolumeText);
  window.draw(m_instructionText);
  window.draw(m_backButtonSprite);
  window.draw(m_backText);
}
} // namespace FishGame
