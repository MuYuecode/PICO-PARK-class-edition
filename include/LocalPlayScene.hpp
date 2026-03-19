//
// Created by cody2 on 2026/3/17.
//

//
// Created for LocalPlayScene
//

#ifndef PICOPART_LOCALPLAYSCENE_HPP
#define PICOPART_LOCALPLAYSCENE_HPP

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"

#include "UI_Triangle_Button.hpp"

class MenuScene;
class KeyboardConfigScene;
class LocalPlayGameScene;

/**
 * @brief 本機多人遊玩人數選擇場景
 *
 * 從 MenuScene 的 LOCAL PLAY 選項進入。
 * 借用 MenuScene 的 m_MenuFrame、m_ExitGameButton、
 * m_LeftTriButton、m_RightTriButton（位置大小不變）。
 *
 * 中央顯示 "nPLAYER GAME"（n = 2-8）。
 * 若 n 超過 KeyboardConfigScene 中已設定按鍵的玩家數：
 *   - 主文字變紅色
 *   - 上方顯示較小的紅色文字 "No keyboard config"
 */
class LocalPlayScene : public Scene {
public:
    LocalPlayScene(GameContext& ctx,
                   MenuScene* menuScene,
                   std::shared_ptr<Character>          menuFrame,
                   std::shared_ptr<Character>          exitGameButton,
                   std::shared_ptr<UI_Triangle_Button> leftTriButton,
                   std::shared_ptr<UI_Triangle_Button> rightTriButton,
                   KeyboardConfigScene* kbConfigScene);
    ~LocalPlayScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetMenuScene(MenuScene* s) { m_MenuScene = s; }
    void SetGameScene(LocalPlayGameScene* s) { m_GameScene = s; }

    /** 回傳目前選擇的玩家人數（2-8） */
    int GetPlayerCount() const { return m_PlayerCount; }

    static constexpr int MIN_PLAYERS = 2;
    static constexpr int MAX_PLAYERS = 8;

private:
    // ── 借用自 MenuScene 的共用 UI ──────────────────────────────────────────
    std::shared_ptr<Character>          m_MenuFrame;
    std::shared_ptr<Character>          m_ExitGameButton;
    std::shared_ptr<UI_Triangle_Button> m_LeftTriButton;
    std::shared_ptr<UI_Triangle_Button> m_RightTriButton;

    // ── 本場景私有 UI ────────────────────────────────────────────────────────
    std::shared_ptr<GameText> m_PlayerCountText;  // "nPLAYER GAME"
    std::shared_ptr<GameText> m_NoConfigText;     // "No keyboard config"

    // ── 狀態 ────────────────────────────────────────────────────────────────
    int m_PlayerCount = MIN_PLAYERS;

    // ── 切換目標 ────────────────────────────────────────────────────────────
    MenuScene*           m_MenuScene     = nullptr;
    KeyboardConfigScene* m_KbConfigScene = nullptr;
    LocalPlayGameScene*  m_GameScene     = nullptr;

    // ── 色彩常數 ────────────────────────────────────────────────────────────
    static const Util::Color k_Black;
    static const Util::Color k_Red;

    // ── 輔助 ─────────────────────────────────────────────────────────────────
    /** 根據 m_PlayerCount 更新文字內容與顏色 */
    void UpdateDisplay();
};

#endif // PICOPART_LOCALPLAYSCENE_HPP