//
// Created by cody2 on 2026/3/20.
//

#include "services/audio/BGMPlayer.hpp"
#include <SDL_mixer.h>
#include "Util/Logger.hpp"

BGMPlayer::BGMPlayer() {
    s_Instance = this;

    m_BGMs.emplace_back(std::make_unique<Util::BGM>(GA_RESOURCE_DIR "/BGM/ppc.mp3"));
    m_BGMs.emplace_back(std::make_unique<Util::BGM>(GA_RESOURCE_DIR "/BGM/pp1.mp3"));
    m_BGMs.emplace_back(std::make_unique<Util::BGM>(GA_RESOURCE_DIR "/BGM/pp2.mp3"));

    for (auto& bgm : m_BGMs) {
        bgm->SetVolume(60);
    }

    Mix_HookMusicFinished(MusicFinishedCallback);
    LOG_INFO("BGMPlayer initialized with 3 tracks");
}

void BGMPlayer::Play() const {
    if (m_BGMs.empty()) return;
    m_BGMs[m_CurrentIndex]->Play(1);
    LOG_INFO("BGM play");
}

void BGMPlayer::Pause() const {
    if (!m_BGMs.empty()) m_BGMs[m_CurrentIndex]->Pause();
}

void BGMPlayer::Resume() const {
    if (!m_BGMs.empty()) m_BGMs[m_CurrentIndex]->Resume();
}

void BGMPlayer::SetVolume(int volume) const {
    for (auto& bgm : m_BGMs) {
        bgm->SetVolume(volume);
    }
}

int BGMPlayer::GetVolume() const {
    return m_BGMs.empty() ? 0 : m_BGMs[m_CurrentIndex]->GetVolume();
}

void BGMPlayer::Next() {
    if (m_BGMs.empty()) return;
    m_CurrentIndex = (m_CurrentIndex + 1) % m_BGMs.size();
    m_BGMs[m_CurrentIndex]->Play(1);
}

void BGMPlayer::MusicFinishedCallback() {
    s_ShouldPlayNext = true ;
}

void BGMPlayer::Update() {
    if (s_ShouldPlayNext) {
        s_ShouldPlayNext = false ;
        Next() ;
    }
}
