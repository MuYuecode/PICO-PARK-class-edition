//
// Created by cody2 on 2026/3/15.
//

#ifndef PICOPART_TITLESCENE_HPP
#define PICOPART_TITLESCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"

// 前向宣告：不 include 完整 header，減少編譯依賴
class MenuScene ;

/**
 * @brief 標題場景（原 STATE_00）
 *
 * 私有 UI 元件：
 *   m_Header          → header.png 大標題
 *   m_TitleSub        → "- CLASSIC EDITED -"
 *   m_PressEnterText  → "PRESS ENTER KEY"（每秒閃爍）
 *
 * 共用物件（來自 GameContext，不在這裡建立）：
 *   m_Ctx.BlueCat / m_Ctx.RedCat  → 貓咪（這個場景允許玩家控制）
 *   m_Ctx.Floor                   → 傳入貓咪 Update 做碰撞
 */
class TitleScene : public Scene {
public:
    /**
     * @param ctx       共用資料（由 App 傳入 reference）
     * @param menuScene App 持有的 MenuScene，此處只借用指標
     */
    TitleScene(GameContext& ctx, MenuScene* menuScene);
    ~TitleScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

private:
    // ---- 此場景私有的 UI 元件 ----
    std::shared_ptr<Character> m_Header;
    std::shared_ptr<GameText>  m_TitleSub;
    std::shared_ptr<GameText>  m_PressEnterText;

    // ---- 閃爍計時器（純粹是 TitleScene 的私有狀態）----
    float m_FlashTimer = 0.0f;

    // ---- 切換目標（non-owning，由 App 持有）----
    MenuScene* m_MenuScene = nullptr;
};

#endif //PICOPART_TITLESCENE_HPP