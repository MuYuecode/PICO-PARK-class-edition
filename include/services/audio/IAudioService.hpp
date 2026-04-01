#ifndef I_AUDIO_SERVICE_HPP
#define I_AUDIO_SERVICE_HPP

class IAudioService {
public:
    virtual ~IAudioService() = default;

    virtual void PlayBgm() = 0;
    virtual void PauseBgm() = 0;
    virtual void ResumeBgm() = 0;
    virtual void UpdateBgm() = 0;

    virtual void SetBgmVolume(int volume) = 0;
    virtual int GetBgmVolume() const = 0;
};

#endif // I_AUDIO_SERVICE_HPP


