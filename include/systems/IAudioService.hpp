#ifndef I_AUDIO_SERVICE_HPP
#define I_AUDIO_SERVICE_HPP

class IAudioService {
public:
    virtual ~IAudioService() = default;

    virtual void PlayBgm() = 0;
    virtual void UpdateBgm() = 0;

    virtual void SetBgmVolume(int volume) = 0;
};

#endif // I_AUDIO_SERVICE_HPP


