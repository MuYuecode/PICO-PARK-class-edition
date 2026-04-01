#ifndef AUDIO_SERVICE_HPP
#define AUDIO_SERVICE_HPP

#include <memory>

#include "services/audio/BGMPlayer.hpp"
#include "services/audio/IAudioService.hpp"

class AudioService final : public IAudioService {
public:
    explicit AudioService(std::shared_ptr<BGMPlayer> bgmPlayer)
        : m_BgmPlayer(std::move(bgmPlayer)) {}

    void PlayBgm() override;
    void PauseBgm() override;
    void ResumeBgm() override;
    void UpdateBgm() override;
    void SetBgmVolume(int volume) override;
    int GetBgmVolume() const override;

private:
    std::shared_ptr<BGMPlayer> m_BgmPlayer;
};

#endif // AUDIO_SERVICE_HPP


