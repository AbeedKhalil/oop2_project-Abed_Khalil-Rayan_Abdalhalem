#pragma once
#include "MusicPlayer.h"
#include "SoundPlayer.h"

namespace FishGame {
class IAudioPlayer {
public:
    virtual ~IAudioPlayer() = default;
    virtual void playMusic(MusicID theme, bool loop = true) = 0;
    virtual void stopMusic() = 0;
    virtual void setMusicVolume(float volume) = 0;
    virtual float getMusicVolume() const = 0;

    virtual void playSound(SoundEffectID effect) = 0;
    virtual void setSoundVolume(float volume) = 0;
    virtual float getSoundVolume() const = 0;
};
} // namespace FishGame
