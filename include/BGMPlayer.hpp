//
// Created by cody2 on 2026/3/20.
//

#ifndef PICOPART_BGMPLAYER_HPP
#define PICOPART_BGMPLAYER_HPP

#include "Util/BGM.hpp"
#include <vector>
#include <string>
#include <memory>

/**
 * @class BGMPlayer
 * @brief 專門管理背景音樂輪播的 OOP 封裝類別
 *
 * 功能：
 * - 自動載入 Resources/BGM/ 資料夾下的三首歌
 * - 自動輪播
 * - 預設音量 50% (64)
 * - 可隨時暫停、繼續、調整音量
 */
class BGMPlayer {
public:
    BGMPlayer();

    void Play() const;                // 播放
    void Pause() const;               // 暫停
    void Resume() const;              // 繼續
    void SetVolume(int volume) const; // 0 ~ 128
    void Next() ;                     // 手動切下一首

    [[nodiscard]] int GetVolume() const; // 取得目前播放的曲目名稱

private:
    static void MusicFinishedCallback();

    std::vector<std::unique_ptr<Util::BGM>> m_BGMs ;
    std::size_t m_CurrentIndex = 0;

    static inline BGMPlayer* s_Instance = nullptr;
};


#endif //PICOPART_BGMPLAYER_HPP