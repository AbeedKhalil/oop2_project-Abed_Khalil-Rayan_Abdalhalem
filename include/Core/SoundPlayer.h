#pragma once

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

namespace FishGame {

enum class SoundEffectID {
    Bite1,
    Bite2,
    Bite3,
    Bite4,
    FreezePowerup,
    LifePowerup,
    MineExplode,
    MouseDown,
    MouseOver,
    OysterPearl,
    PlayerGrow,
    PlayerPoison,
    PlayerSpawn,
    PlayerStunned,
    PufferBounce,
    SpeedEnd,
    SpeedStart,
    StageIntro,
    StarPickup
};

class SoundPlayer {
public:
    SoundPlayer();

    void play(SoundEffectID effect);
    void setVolume(float volume);

private:
    using BufferPtr = std::unique_ptr<sf::SoundBuffer>;

    std::unordered_map<SoundEffectID, BufferPtr> m_soundBuffers;
    std::unordered_map<SoundEffectID, std::string> m_filenames;
    std::vector<sf::Sound> m_sounds;
    float m_volume;
};

}

