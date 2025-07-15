#pragma once
#include "IAudioPlayer.h"
#include <memory>

namespace FishGame {
class DefaultAudioPlayer : public IAudioPlayer {
public:
    DefaultAudioPlayer();
    ~DefaultAudioPlayer() override = default;

    void playMusic(MusicID theme, bool loop = true) override;
    void stopMusic() override;
    void setMusicVolume(float volume) override;
    float getMusicVolume() const override;

    void playSound(SoundEffectID effect) override;
    void setSoundVolume(float volume) override;
    float getSoundVolume() const override;

private:
    std::unique_ptr<MusicPlayer> m_musicPlayer;
    std::unique_ptr<SoundPlayer> m_soundPlayer;
};
}
