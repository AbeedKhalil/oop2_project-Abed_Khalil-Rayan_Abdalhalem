#include "DefaultAudioPlayer.h"

namespace FishGame {

DefaultAudioPlayer::DefaultAudioPlayer()
    : m_musicPlayer(std::make_unique<MusicPlayer>()),
      m_soundPlayer(std::make_unique<SoundPlayer>()) {}

void DefaultAudioPlayer::playMusic(MusicID theme, bool loop) {
    m_musicPlayer->play(theme, loop);
}

void DefaultAudioPlayer::stopMusic() { m_musicPlayer->stop(); }

void DefaultAudioPlayer::setMusicVolume(float volume) {
    m_musicPlayer->setVolume(volume);
}

float DefaultAudioPlayer::getMusicVolume() const {
    return m_musicPlayer->getVolume();
}

void DefaultAudioPlayer::playSound(SoundEffectID effect) {
    m_soundPlayer->play(effect);
}

void DefaultAudioPlayer::setSoundVolume(float volume) {
    m_soundPlayer->setVolume(volume);
}

float DefaultAudioPlayer::getSoundVolume() const {
    return m_soundPlayer->getVolume();
}

} // namespace FishGame
