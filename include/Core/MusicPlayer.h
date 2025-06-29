#pragma once

#include <SFML/Audio.hpp>
#include <string>
#include <thread>
#include <unordered_map>

namespace FishGame {

enum class MusicID {
  MenuTheme,
  InGame1,
  InGame2,
  InGame3,
  BonusStage,
  InstructionsHelp,
  ScoreSummary,
  StageCleared,
  PlayerDies
};

class MusicPlayer {
public:
  MusicPlayer();

  void play(MusicID theme, bool loop = true);
  void stop();
  void setVolume(float volume);
  float getVolume() const { return m_volume; }

  static constexpr float FadeDuration = 0.5f; ///< seconds

private:
  using MusicPtr = std::unique_ptr<sf::Music>;

  /// Pre-loaded music tracks for fast playback
  std::unordered_map<MusicID, MusicPtr> m_musicTracks;

  /// Mapping from music ID to file name
  std::unordered_map<MusicID, std::string> m_filenames;

  /// Currently playing music (non-owning pointer)
  sf::Music *m_current;

  /// Background cross fade thread
  std::jthread m_fadeThread;

  float m_volume;
};

} // namespace FishGame
