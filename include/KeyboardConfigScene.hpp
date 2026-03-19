//
// Created by cody2 on 2026/3/16.
//

//
// Created by cody2 on 2026/3/16.
//

#ifndef PICOPART_KEYBOARDCONFIGSCENE_HPP
#define PICOPART_KEYBOARDCONFIGSCENE_HPP

#include <array>
#include <vector>
#include <string>

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"
#include "UI_Triangle_Button.hpp"
#include "Util/Keycode.hpp"

class OptionMenuScene;

// ────────────────────────────────────────────────────────────────────────────
// 一位玩家的鍵盤設定組合
// ────────────────────────────────────────────────────────────────────────────
struct PlayerKeyConfig {
    Util::Keycode up         = Util::Keycode::UNKNOWN;
    Util::Keycode down       = Util::Keycode::UNKNOWN;
    Util::Keycode left       = Util::Keycode::UNKNOWN;
    Util::Keycode right      = Util::Keycode::UNKNOWN;
    Util::Keycode jump       = Util::Keycode::UNKNOWN;
    Util::Keycode cancel     = Util::Keycode::UNKNOWN;
    Util::Keycode shot       = Util::Keycode::UNKNOWN;
    Util::Keycode menu       = Util::Keycode::UNKNOWN;  // 僅 1P 有效
    Util::Keycode subMenu    = Util::Keycode::UNKNOWN;  // 僅 1P 有效

    // 回傳此 config 中所有非 UNKNOWN 的按鍵（用於衝突偵測）
    std::vector<Util::Keycode> AllKeys() const;
};

// ────────────────────────────────────────────────────────────────────────────
// KeyboardConfigScene
// ────────────────────────────────────────────────────────────────────────────
class KeyboardConfigScene : public Scene {
public:
    KeyboardConfigScene(GameContext& ctx,
                        OptionMenuScene* optionScene,
                        std::shared_ptr<Character> exitGameButton);
    ~KeyboardConfigScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetOptionScene(OptionMenuScene* s) { m_OptionScene = s; }

    // ── 預設設定（供外部讀取，或未來存檔使用） ────────────────────────────
    static const PlayerKeyConfig k_Default1P;
    static const PlayerKeyConfig k_Default2P;

    static constexpr int MAX_PLAYERS = 8;

    /**
     * @brief 回傳已設定足夠按鍵（≥4 個非 UNKNOWN）的玩家數量。
     *        供 LocalPlayScene 判斷是否顯示 "No keyboard config" 警告。
     */
    int GetConfiguredPlayerCount() const;

    [[nodiscard]] PlayerKeyConfig GetAppliedConfig(int playerIdx) const {
        if (playerIdx < 0 || playerIdx >= MAX_PLAYERS) {
            return PlayerKeyConfig{};
        }
        return m_Applied[playerIdx];
    }

private:
    // ── 借用物件 ───────────────────────────────────────────────────────────
    std::shared_ptr<Character> m_ExitGameButton;

    // ── 專屬框架 ───────────────────────────────────────────────────────────
    std::shared_ptr<Character> m_Frame;

    // ── 選擇框 ─────────────────────────────────────────────────────────────
    std::shared_ptr<Character> m_ChoiceFrame;

    // ── 橫線分隔（×3）─────────────────────────────────────────────────────
    std::shared_ptr<Character> m_HLine1;
    std::shared_ptr<Character> m_HLine2;
    std::shared_ptr<Character> m_HLine3;

    // ── 標題 ───────────────────────────────────────────────────────────────
    std::shared_ptr<GameText> m_TitleText;

    // ── PLAYER 列 ──────────────────────────────────────────────────────────
    std::shared_ptr<GameText>           m_PlayerLabel;
    std::shared_ptr<UI_Triangle_Button> m_PlayerLeftBtn;
    std::shared_ptr<GameText>           m_PlayerValue;   // "1P" ~ "8P"
    std::shared_ptr<UI_Triangle_Button> m_PlayerRightBtn;

    // ── 按鍵綁定列（每列：label + value）──────────────────────────────────
    static constexpr int BIND_COUNT = 9;
    std::array<std::shared_ptr<GameText>, BIND_COUNT> m_BindLabels;
    std::array<std::shared_ptr<GameText>, BIND_COUNT> m_BindValues;

    // ── 底部按鈕 ───────────────────────────────────────────────────────────
    std::shared_ptr<GameText> m_OkText;
    std::shared_ptr<GameText> m_CancelText;
    std::shared_ptr<GameText> m_DefaultText;

    // ── 切換目標 ───────────────────────────────────────────────────────────
    OptionMenuScene* m_OptionScene = nullptr;

    // ── 設定狀態 ───────────────────────────────────────────────────────────
    std::array<PlayerKeyConfig, MAX_PLAYERS> m_Applied;
    PlayerKeyConfig m_Pending;

    int  m_CurrentPlayer = 0;

    // ── 游標 ───────────────────────────────────────────────────────────────
    int  m_SelectedRow  = 0;
    static constexpr int ROW_PLAYER   = 0;
    static constexpr int ROW_BIND_0   = 1;
    static constexpr int ROW_MENU_KEY = 8;
    static constexpr int ROW_SUBMENU  = 9;
    static constexpr int ROW_OK       = 10;
    static constexpr int ROW_CANCEL   = 11;
    static constexpr int ROW_DEFAULT  = 12;
    static constexpr int ROW_COUNT    = 13;

    // ── 按鍵捕捉模式 ───────────────────────────────────────────────────────
    bool m_WaitingForKey = false;
    int  m_WaitingRow    = -1;

    // ── 布局常數 ───────────────────────────────────────────────────────────
    static constexpr float COL_LABEL_X     = -295.0f;
    static constexpr float COL_LEFT_BTN_X  =  130.0f;
    static constexpr float COL_VALUE_X     =  245.0f;
    static constexpr float COL_RIGHT_BTN_X =  340.0f;
    static constexpr float COL_OK_X        = -170.0f;
    static constexpr float COL_CANCEL_X    =   10.0f;
    static constexpr float COL_DEFAULT_X   =  175.0f;

    static constexpr float ROW_Y_TITLE    =  242.0f;
    static constexpr float ROW_Y_HLINE1   =  208.0f;
    static constexpr float ROW_Y_PLAYER   =  170.0f;
    static constexpr float ROW_Y_HLINE2   =  135.0f;
    static constexpr float ROW_Y_BIND_TOP =   96.0f;
    static constexpr float ROW_Y_STEP     =  -37.5f;
    static constexpr float ROW_Y_HLINE3   = -240.5f;
    static constexpr float ROW_Y_BTN      = -274.5f;

    // ── 色彩 ───────────────────────────────────────────────────────────────
    static const Util::Color k_Black;
    static const Util::Color k_Red;
    static const Util::Color k_Grey;

    // ── 輔助函式 ───────────────────────────────────────────────────────────
    void LoadPlayer(int playerIdx);
    void CommitPending();
    void ApplyDefault();

    void DecrementRow();
    void IncrementRow();
    bool IsRowSelectable(int row) const;

    void AssignKey(int bindIdx, Util::Keycode key);
    Util::Keycode GetPendingKey(int bindIdx) const;

    void UpdateValueTexts();
    void UpdateChoiceFrame();

    std::vector<Util::Keycode> GetConflicts() const;
    bool HasConflicts() const { return !GetConflicts().empty(); }

    static float BindRowY(int bindIdx) {
        return ROW_Y_BIND_TOP + bindIdx * ROW_Y_STEP;
    }
    float RowY(int row) const;
};

#endif // PICOPART_KEYBOARDCONFIGSCENE_HPP