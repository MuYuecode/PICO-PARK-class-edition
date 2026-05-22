#ifndef AUDIO_SERVICE_HPP
#define AUDIO_SERVICE_HPP

#include <memory>

#include "systems/BGMPlayer.hpp"
#include "systems/IAudioService.hpp"

class AudioService final : public IAudioService {
public:
    explicit AudioService(std::shared_ptr<BGMPlayer> bgmPlayer)
        : m_BgmPlayer(std::move(bgmPlayer)) {}

    void PlayBgm() override;
    void UpdateBgm() override;
    void SetBgmVolume(int volume) override;

private:
    std::shared_ptr<BGMPlayer> m_BgmPlayer;
};

#endif // AUDIO_SERVICE_HPP


