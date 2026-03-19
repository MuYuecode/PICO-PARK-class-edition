//
// Created by cody2 on 2026/3/15.
//

#ifndef PICOPART_OPTIONMENUSCENE_HPP
#define PICOPART_OPTIONMENUSCENE_HPP

#include <vector>
#include <string>

#include "Scene.hpp"
#include "Character.hpp"
#include "GameText.hpp"
#include "UI_Triangle_Button.hpp"

class MenuScene;
class KeyboardConfigScene;

/**
 * @brief 遊戲設定選單場景
 *
 * 選單結構（由上至下）：
 *   OPTION（標題，置中）
 *   KEYBOARD CONFIG  |  OPEN
 *   BG COLOR         |  ◀  WHITE  ▶
 *   BGM VOLUME       |  ◀  0      ▶
 *   SE VOLUME        |  ◀  0      ▶
 *   DISP NUMBER      |  ◀  OFF    ▶
 *   ──────────────────────────────
 *          OK              CANCEL
 *
 * 操作方式：
 *   W / ↑  : 游標上移
 *   S / ↓  : 游標下移
 *   A / 左鍵點擊左方按鈕 : 目前列左調整
 *   D / 左鍵點擊右方按鈕 : 目前列右調整
 *   ENTER  : 確認（KEYBOARD CONFIG 開啟子頁；OK/CANCEL 離開）
 *   ESC / X 按鈕 / CANCEL : 取消並返回 MenuScene
 *
 * m_ChoiceFrame 永遠圍住當前選中列的「值」區域（OPEN 或中間文字）。
 */
class OptionMenuScene : public Scene {
public:
    OptionMenuScene(GameContext& ctx,
                    MenuScene* menuScene,
                    std::shared_ptr<Character> exitGameButton);
    ~OptionMenuScene() override = default;

    void   OnEnter() override;
    void   OnExit()  override;
    Scene* Update()  override;

    void SetMenuScene(MenuScene* s) { m_MenuScene = s; }
    void SetKeyboardConfigScene(KeyboardConfigScene* s) { m_KeyboardConfigScene = s; }

    struct Settings {
        int  bgColorIndex = 0;
        int  bgmVolume    = 0;
        int  seVolume     = 0;
        bool dispNumber   = false;
    };

    Settings m_Applied;   // 真正生效的設定（OK 才寫入）
    Settings m_Pending;   // 選單開啟期間的暫時操作(會顯示在選單UI上)

private:
    // ── 與 MenuScene 共用（借用，不持有所有權）────────────────────────────
    std::shared_ptr<Character> m_ExitGameButton;

    // ── 大框架 ─────────────────────────────────────────────────────────────
    std::shared_ptr<Character> m_OptionMenuFrame;

    // ── 選擇框 ─────────────────────────────────────────────────────────────
    std::shared_ptr<Character> m_ChoiceFrame;

    // ── 標題 ───────────────────────────────────────────────────────────────
    std::shared_ptr<GameText> m_TitleText;           // "OPTION"

    // ── KEYBOARD CONFIG 列 ─────────────────────────────────────────────────
    std::shared_ptr<GameText> m_KbConfigLabel;       // "KEYBOARD CONFIG"
    std::shared_ptr<GameText> m_KbConfigOpen;        // "OPEN"

    // ── BG COLOR 列 ────────────────────────────────────────────────────────
    std::shared_ptr<GameText>           m_BgColorLabel;
    std::shared_ptr<UI_Triangle_Button> m_BgColorLeftBtn;
    std::shared_ptr<GameText>           m_BgColorValue;
    std::shared_ptr<UI_Triangle_Button> m_BgColorRightBtn;

    // ── BGM VOLUME 列 ──────────────────────────────────────────────────────
    std::shared_ptr<GameText>           m_BgmVolumeLabel;
    std::shared_ptr<UI_Triangle_Button> m_BgmVolumeLeftBtn;
    std::shared_ptr<GameText>           m_BgmVolumeValue;
    std::shared_ptr<UI_Triangle_Button> m_BgmVolumeRightBtn;

    // ── SE VOLUME 列 ───────────────────────────────────────────────────────
    std::shared_ptr<GameText>           m_SeVolumeLabel;
    std::shared_ptr<UI_Triangle_Button> m_SeVolumeLeftBtn;
    std::shared_ptr<GameText>           m_SeVolumeValue;
    std::shared_ptr<UI_Triangle_Button> m_SeVolumeRightBtn;

    // ── DISP NUMBER 列 ─────────────────────────────────────────────────────
    std::shared_ptr<GameText>           m_DispNumberLabel;
    std::shared_ptr<UI_Triangle_Button> m_DispNumberLeftBtn;
    std::shared_ptr<GameText>           m_DispNumberValue;
    std::shared_ptr<UI_Triangle_Button> m_DispNumberRightBtn;

    // ── 底部按鈕 ───────────────────────────────────────────────────────────
    std::shared_ptr<GameText> m_OkText;
    std::shared_ptr<GameText> m_CancelText;

    // ── 切換目標 ───────────────────────────────────────────────────────────
    MenuScene* m_MenuScene = nullptr;
    KeyboardConfigScene* m_KeyboardConfigScene = nullptr;

    // ── 選單值狀態 ─────────────────────────────────────────────────────────
    // 可擴充：在 s_BgColorOptions 中新增顏色字串即可
    static const std::vector<std::string> s_BgColorOptions;

    // ── 游標列（0 = KEYBOARD_CONFIG … 6 = CANCEL）─────────────────────────
    int m_SelectedRow   = 0;

    static constexpr int ROW_COUNT = 7;

    // ── 列 Y 座標（螢幕空間，中心點）──────────────────────────────────────
    static constexpr float ROW_Y_KB    =  145.0f;
    static constexpr float ROW_Y_BG    =  60.0f;
    static constexpr float ROW_Y_BGM   = -25.0f;
    static constexpr float ROW_Y_SE    = -110.0f;
    static constexpr float ROW_Y_DISP  = -195.0f;
    static constexpr float ROW_Y_BTN   = -260.0f;

    // ── 欄 X 座標（螢幕空間，中心點）──────────────────────────────────────
    // 左方標籤欄
    static constexpr float COL_LABEL_X      = -330.0f;
    // 右方：左三角按鈕、中間值文字、右三角按鈕
    static constexpr float COL_LEFT_BTN_X   =  100.0f;
    static constexpr float COL_VALUE_X      =  230.0f;
    static constexpr float COL_RIGHT_BTN_X  =  330.0f;
    // 底部 OK / CANCEL（對稱排列）
    static constexpr float COL_OK_X         = -130.0f;
    static constexpr float COL_CANCEL_X     =  130.0f;

    // ── 輔助函式 ───────────────────────────────────────────────────────────
    void UpdateChoiceFrame();
    void UpdateValueTexts();

    void DecrementRow();
    void IncrementRow();
    void AdjustLeft(int row);
    void AdjustRight(int row);
};

#endif //PICOPART_OPTIONMENUSCENE_HPP