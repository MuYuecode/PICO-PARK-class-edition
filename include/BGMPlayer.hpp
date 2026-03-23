//
// Created by cody2 on 2026/3/20.
//

#ifndef PICOPART_BGMPLAYER_HPP
#define PICOPART_BGMPLAYER_HPP

#include "Util/BGM.hpp"
#include <vector>
#include <memory>


class BGMPlayer {
public:
    BGMPlayer();

    void Play() const;                // 播放
    void Pause() const;               // 暫停
    void Resume() const;              // 繼續
    void SetVolume(int volume) const; // 0 ~ 128
    void Next() ;                     // 手動切下一首
    void Update();                    // 更新

    [[nodiscard]] int GetVolume() const; // 取得目前播放的曲目名稱

    static inline bool s_ShouldPlayNext = false;

private:
    static void MusicFinishedCallback();


    std::vector<std::unique_ptr<Util::BGM>> m_BGMs ;
    std::size_t m_CurrentIndex = 0;

    static inline BGMPlayer* s_Instance = nullptr;
};


#endif //PICOPART_BGMPLAYER_HPP