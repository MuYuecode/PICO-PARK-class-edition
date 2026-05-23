#ifndef AUDIO_SERVICE_HPP
#define AUDIO_SERVICE_HPP

#include <array>
#include <cstddef>
#include <memory>

#include "systems/BGMPlayer.hpp"
#include "systems/IAudioService.hpp"

namespace Util {
class SFX;
}

class AudioService final : public IAudioService {
public:
    explicit AudioService(std::shared_ptr<BGMPlayer> bgmPlayer);
    ~AudioService() override;

    void PlayBgm() override;
    void UpdateBgm() override;
    void SetBgmVolume(int volume) override;
    void SetSeVolume(int volume) override;
    void PlaySe(SoundEffect effect) override;

private:
    static constexpr std::size_t kSeCount = static_cast<std::size_t>(SoundEffect::Count);

    std::shared_ptr<BGMPlayer> m_BgmPlayer;
    std::array<std::unique_ptr<Util::SFX>, kSeCount> m_SoundEffects;
    int m_SeVolume = 60;
};

#endif // AUDIO_SERVICE_HPP


