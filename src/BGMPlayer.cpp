//
// Created by cody2 on 2026/3/20.
//

#include "BGMPlayer.hpp"
#include <SDL_mixer.h>
#include "Util/Logger.hpp"

BGMPlayer::BGMPlayer() {
    s_Instance = this;

    // ==================== 載入三首歌 ====================
    m_BGMs.emplace_back(std::make_unique<Util::BGM>(GA_RESOURCE_DIR "/BGM/ppc.mp3"));
    m_BGMs.emplace_back(std::make_unique<Util::BGM>(GA_RESOURCE_DIR "/BGM/pp1.mp3"));
    m_BGMs.emplace_back(std::make_unique<Util::BGM>(GA_RESOURCE_DIR "/BGM/pp2.mp3"));

    // ==================== 預設音量 50% ====================
    for (auto& bgm : m_BGMs) {
        bgm->SetVolume(60);
    }

    // ==================== 音樂結束自動切歌 ====================

    Mix_HookMusicFinished(MusicFinishedCallback);
    LOG_INFO("BGMPlayer initialized with 3 tracks");
}

// ====================== 公開介面 ======================

void BGMPlayer::Play() const {
    if (m_BGMs.empty()) return;
    m_BGMs[m_CurrentIndex]->Play(0);
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
    m_BGMs[m_CurrentIndex]->Play(0);
}

// ==================== 靜態回呼 ====================
void BGMPlayer::MusicFinishedCallback() {
    if (s_Instance) {
        s_Instance->Next();
    }
}