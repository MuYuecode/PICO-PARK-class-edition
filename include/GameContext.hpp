#ifndef PICOPART_GAMECONTEXT_HPP
#define PICOPART_GAMECONTEXT_HPP

#include "pch.hpp" // IWYU pragma: export
#include "Util/Renderer.hpp"
#include "Character.hpp"
#include "GameText.hpp"    // [新增] TitleSub 是 GameText
#include "PlayerCat.hpp"

struct GameContext {
    Util::Renderer& Root;

    // 永久留在渲染樹的物件（所有場景都看得到）
    std::shared_ptr<Character>  WhiteBackground;
    std::shared_ptr<Character>  Floor;

    // [新增] Header 在 TitleScene 和 MenuScene 都要顯示，
    // 所以移到 GameContext，由 AppStart 加入渲染樹後就不再移除。
    std::shared_ptr<Character>  Header;

    std::shared_ptr<PlayerCat>  BlueCat;
    std::shared_ptr<PlayerCat>  RedCat;

    bool ShouldQuit = false;

    explicit GameContext(Util::Renderer& root)
        : Root(root) {}

    GameContext(const GameContext&)            = delete;
    GameContext& operator=(const GameContext&) = delete;
    GameContext(GameContext&&)                 = delete;
    GameContext& operator=(GameContext&&)      = delete;
};

#endif //PICOPART_GAMECONTEXT_HPP