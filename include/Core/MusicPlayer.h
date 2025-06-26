#pragma once

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>

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

private:
    sf::Music m_music;
    std::unordered_map<MusicID, std::string> m_filenames;
    float m_volume;
};

}
