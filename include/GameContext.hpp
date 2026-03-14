//
// Created by cody2 on 2026/3/14.
//

#ifndef PICOPART_GAMECONTEXT_HPP
#define PICOPART_GAMECONTEXT_HPP

#include "pch.hpp" // IWYU pragma: export
#include "Util/Renderer.hpp"
#include "Character.hpp"
#include "PlayerCat.hpp"

/**
 * @brief 跨場景共用的資料容器 (Shared Blackboard)
 *
 * 放入原則：某物件在「場景 A 建立，但場景 B 也需要操作」→ 放這裡
 * 場景私有的 UI 元件（按鈕、文字框）→ 不應該放這裡
 *
 * 所有權：App 以 unique_ptr<GameContext> 持有唯一一份
 */
struct GameContext {
    // -----------------------------------------------------------------------
    // Renderer：唯一一份，所有場景共用同一棵渲染樹
    // 使用 reference，因為 App 才是真正的擁有者
    // -----------------------------------------------------------------------
    Util::Renderer& Root;

    // -----------------------------------------------------------------------
    // 跨場景共用物件
    // -----------------------------------------------------------------------
    std::shared_ptr<Character>  WhiteBackground;
    std::shared_ptr<Character>  Floor;
    std::shared_ptr<PlayerCat>  BlueCat;
    std::shared_ptr<PlayerCat>  RedCat;

    // -----------------------------------------------------------------------
    // 遊戲結束旗標
    //
    // ExitConfirmScene 把 ShouldQuit 設成 true，App::Update 負責檢查。
    // 這樣 Scene 不需要持有 App* 指標，避免循環依賴。
    // -----------------------------------------------------------------------
    bool ShouldQuit = false;

    // -----------------------------------------------------------------------
    // 建構子：Root 以 reference 綁定，其餘在 App::Start() 裡填入
    // -----------------------------------------------------------------------
    explicit GameContext(Util::Renderer& root)
        : Root(root) {}

    // reference member 存在，禁止複製與移動
    GameContext(const GameContext&)            = delete;
    GameContext& operator=(const GameContext&) = delete;
    GameContext(GameContext&&)                 = delete;
    GameContext& operator=(GameContext&&)      = delete;
};

#endif //PICOPART_GAMECONTEXT_HPP