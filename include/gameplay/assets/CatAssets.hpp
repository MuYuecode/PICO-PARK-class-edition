//
// Created by cody2 on 2026/3/23.
//

//
// CatAssets.hpp
// 貓咪資源路徑工具(統一出口)
//
// 解決【重複 1】：AppStart.cpp 與 LocalPlayGameScene.cpp 各自實作
// BuildCatFramePath / BuildCatFramePaths / BuildCatAnimPaths 的問題
// 兩處改為共同 #include 此標頭，並呼叫對應的函式
//
#ifndef CAT_ASSETS_HPP
#define CAT_ASSETS_HPP

#include <string>
#include <vector>
#include "gameplay/actors/PlayerCat.hpp"   // CatAnimPaths

namespace CatAssets {
inline std::string BuildFramePath(const std::string& color,
                                  const std::string& action,
                                  int frameNum) {
    return std::string(GA_RESOURCE_DIR) +
           "/Image/Character/" + color + "_cat/" +
           color + "_cat_" + action + "_" + std::to_string(frameNum) + ".png";
}

inline std::vector<std::string> BuildFramePaths(const std::string& color,
                                                 const std::string& action,
                                                 int numFrames) {
    std::vector<std::string> paths;
    paths.reserve(static_cast<size_t>(numFrames));
    for (int f = 1; f <= numFrames; ++f) {
        paths.push_back(BuildFramePath(color, action, f));
    }
    return paths;
}
inline CatAnimPaths BuildFullAnimPaths(const std::string& color) {
    CatAnimPaths p;
    p.stand     = BuildFramePaths(color, "stand", 8);
    p.run       = BuildFramePaths(color, "run",   9);
    p.jump_rise = { BuildFramePath(color, "jump", 1) };
    p.jump_fall = { BuildFramePath(color, "jump", 2) };
    p.land      = { BuildFramePath(color, "land", 1) };
    p.push      = BuildFramePaths(color, "push",  3);
    return p;
}
}

#endif // CAT_ASSETS_HPP
