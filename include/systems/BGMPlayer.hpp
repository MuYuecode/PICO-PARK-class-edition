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

    void Play() const;
    void SetVolume(int volume) const; // 0 ~ 128
    void Next() ;
    void Update();

    static inline bool s_ShouldPlayNext = false;

private:
    static void MusicFinishedCallback();


    std::vector<std::unique_ptr<Util::BGM>> m_BGMs ;
    std::size_t m_CurrentIndex = 0;

    static inline BGMPlayer* s_Instance = nullptr;
};


#endif //PICOPART_BGMPLAYER_HPP
