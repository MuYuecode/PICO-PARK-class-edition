#ifndef I_AUDIO_SERVICE_HPP
#define I_AUDIO_SERVICE_HPP

enum class SoundEffect {
    Button,
    Death,
    Door,
    Jump,
    Win,
    Count
};

class IAudioService {
public:
    virtual ~IAudioService() = default;

    virtual void PlayBgm() = 0;
    virtual void UpdateBgm() = 0;

    virtual void SetBgmVolume(int volume) = 0;
    virtual void SetSeVolume(int volume) = 0;
    virtual void PlaySe(SoundEffect effect) = 0;
};

#endif // I_AUDIO_SERVICE_HPP


