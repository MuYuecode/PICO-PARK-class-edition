#include "systems/AudioService.hpp"

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


