//
// Created by cody2 on 2026/3/23.
//

//
// CatAssets.hpp
// 貓咪資源路徑工具（統一出口）
//
// 解決【重複 1】：AppStart.cpp 與 LocalPlayGameScene.cpp 各自實作
// BuildCatFramePath / BuildCatFramePaths / BuildCatAnimPaths 的問題。
// 兩處改為共同 #include 此標頭，並呼叫對應的函式。
//
#ifndef CAT_ASSETS_HPP
#define CAT_ASSETS_HPP

#include <string>
#include <vector>
#include "PlayerCat.hpp"   // CatAnimPaths

namespace CatAssets {
// 建立單幀路徑
// e.g. BuildFramePath("blue", "stand", 1)
//      → ".../Image/Character/blue_cat/blue_cat_stand_1.png"
inline std::string BuildFramePath(const std::string& color,
                                  const std::string& action,
                                  int frameNum) {
    return std::string(GA_RESOURCE_DIR) +
           "/Image/Character/" + color + "_cat/" +
           color + "_cat_" + action + "_" + std::to_string(frameNum) + ".png";
}

// 建立多幀路徑（frame 1 ~ numFrames）
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
// 完整動畫幀數版本
//   stand : 8 幀   run : 9 幀   push : 3 幀
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
} // namespace CatAssets

#endif // CAT_ASSETS_HPP