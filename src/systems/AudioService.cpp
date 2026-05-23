#include "systems/AudioService.hpp"

#include <algorithm>
#include <memory>
#include <utility>

#include "Util/SFX.hpp"

namespace {
std::size_t ToIndex(SoundEffect effect) {
    return static_cast<std::size_t>(effect);
}
}

AudioService::AudioService(std::shared_ptr<BGMPlayer> bgmPlayer)
    : m_BgmPlayer(std::move(bgmPlayer)) {
    m_SoundEffects[ToIndex(SoundEffect::Button)] =
        std::make_unique<Util::SFX>(GA_RESOURCE_DIR "/SE/button.mp3");
    m_SoundEffects[ToIndex(SoundEffect::Death)] =
        std::make_unique<Util::SFX>(GA_RESOURCE_DIR "/SE/death.mp3");
    m_SoundEffects[ToIndex(SoundEffect::Door)] =
        std::make_unique<Util::SFX>(GA_RESOURCE_DIR "/SE/door.mp3");
    m_SoundEffects[ToIndex(SoundEffect::Jump)] =
        std::make_unique<Util::SFX>(GA_RESOURCE_DIR "/SE/jump.mp3");
    m_SoundEffects[ToIndex(SoundEffect::Win)] =
        std::make_unique<Util::SFX>(GA_RESOURCE_DIR "/SE/win.mp3");

    SetSeVolume(m_SeVolume);
}

AudioService::~AudioService() = default;

void AudioService::PlayBgm() {
    if (m_BgmPlayer != nullptr) {
        m_BgmPlayer->Play();
    }
}

void AudioService::UpdateBgm() {
    if (m_BgmPlayer != nullptr) {
        m_BgmPlayer->Update();
    }
}

void AudioService::SetBgmVolume(int volume) {
    if (m_BgmPlayer != nullptr) {
        m_BgmPlayer->SetVolume(volume);
    }
}

void AudioService::SetSeVolume(int volume) {
    m_SeVolume = std::clamp(volume, 0, 128);
    for (auto& effect : m_SoundEffects) {
        if (effect != nullptr) {
            effect->SetVolume(m_SeVolume);
        }
    }
}

void AudioService::PlaySe(SoundEffect effect) {
    const std::size_t index = ToIndex(effect);
    if (index >= m_SoundEffects.size() || m_SoundEffects[index] == nullptr) {
        return;
    }

    m_SoundEffects[index]->Play();
}


