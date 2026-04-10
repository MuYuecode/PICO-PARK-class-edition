#include "services/AudioService.hpp"

void AudioService::PlayBgm() {
    if (m_BgmPlayer != nullptr) {
        m_BgmPlayer->Play();
    }
}

void AudioService::PauseBgm() {
    if (m_BgmPlayer != nullptr) {
        m_BgmPlayer->Pause();
    }
}

void AudioService::ResumeBgm() {
    if (m_BgmPlayer != nullptr) {
        m_BgmPlayer->Resume();
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

int AudioService::GetBgmVolume() const {
    return (m_BgmPlayer != nullptr) ? m_BgmPlayer->GetVolume() : 0;
}


